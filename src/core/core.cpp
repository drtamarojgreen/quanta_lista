#include "core.h"
#include "../utils/json_utils.h"
#include "../models/ModelBackend.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <set>
#include <iomanip>
#include <random>

// --- AgentManager Implementation ---

AgentManager::AgentManager(Publisher& pub) : publisher(pub) {}

void AgentManager::registerAgent(const Agent& agent) {
    agents.emplace(agent.id, agent);
    publisher.publish(AgentStateChangedEvent(agent.id, agent.state));
}

Agent* AgentManager::getIdleAgent() {
    for (auto& pair : agents) {
        if (pair.second.state == AgentState::IDLE && !pair.second.disabled) {
            return &pair.second;
        }
    }
    return nullptr;
}

Agent* AgentManager::getIdleAgentForTask(const Task& task) {
    std::vector<Agent*> candidates;
    for (auto& pair : agents) {
        Agent& agent = pair.second;
        if (agent.state == AgentState::IDLE && !agent.disabled) {
            if (task.component.empty() || std::find(agent.capabilities.begin(), agent.capabilities.end(), task.component) != agent.capabilities.end()) {
                 candidates.push_back(&agent);
            }
        }
    }
    if (candidates.empty()) return nullptr;
    static size_t last_idx = 0;
    last_idx %= candidates.size();
    return candidates[last_idx++];
}

void AgentManager::setAgentState(const std::string& agentId, AgentState newState) {
    auto it = agents.find(agentId);
    if (it != agents.end()) {
        if (it->second.state != newState) {
            it->second.state = newState;
            publisher.publish(AgentStateChangedEvent(agentId, newState));
        }
    }
}

void AgentManager::updateHeartbeat(const std::string& agentId) {
    if (agents.count(agentId)) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        agents.at(agentId).last_heartbeat = ss.str();
    }
}

void AgentManager::disableAgent(const std::string& agentId) {
    if (agents.count(agentId)) {
        agents.at(agentId).disabled = true;
    }
}

void AgentManager::enableAgent(const std::string& agentId) {
    if (agents.count(agentId)) {
        agents.at(agentId).disabled = false;
    }
}

void AgentManager::checkStaleAgents(int timeout_sec) {
    auto now = std::chrono::system_clock::now();
    for (auto& pair : agents) {
        Agent& agent = pair.second;
        if (!agent.last_heartbeat.empty()) {
            std::tm tm = {};
            std::stringstream ss(agent.last_heartbeat);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            if (std::chrono::duration_cast<std::chrono::seconds>(now - tp).count() > timeout_sec) {
                agent.disabled = true;
            }
        }
    }
}

const Agent* AgentManager::getAgent(const std::string& agentId) const {
    auto it = agents.find(agentId);
    if (it != agents.end()) return &it->second;
    return nullptr;
}

// --- Scheduler Implementation ---

Scheduler::Scheduler(Publisher& pub) : publisher(pub), pending_tasks(TaskComparator{&tasks}) {}

void Scheduler::submitTask(const Task& task) {
    tasks[task.task_id] = task;
    pending_tasks.insert(task.task_id);
    publisher.publish(TaskCreatedEvent(task.task_id, task.description));
}

bool Scheduler::areDependenciesMet(const Task& task) {
    for (const auto& dep : task.dependencies) {
        if (std::find(completed_task_ids.begin(), completed_task_ids.end(), dep) == completed_task_ids.end()) return false;
    }
    return true;
}

Task* Scheduler::getNextAvailableTask() {
    if (isCircuitBroken()) return nullptr;
    for (auto it = pending_tasks.begin(); it != pending_tasks.end(); ++it) {
        Task& task = tasks[*it];
        if (std::find(paused_task_ids.begin(), paused_task_ids.end(), *it) != paused_task_ids.end()) continue;
        if (areDependenciesMet(task)) {
            std::string tid = *it;
            in_progress_task_ids.push_back(tid);
            task_start_times[tid] = std::chrono::steady_clock::now();
            publisher.publish(TaskStatusChangedEvent(tid, TaskStatus::InProgress));
            Task* t_ptr = &tasks.at(tid);
            pending_tasks.erase(it);
            return t_ptr;
        }
    }
    return nullptr;
}

void Scheduler::markTaskAsCompleted(const std::string& taskId) {
    auto it = std::find(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId);
    if (it != in_progress_task_ids.end()) {
        in_progress_task_ids.erase(it);
        if (task_start_times.count(taskId)) {
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = end - task_start_times[taskId];
            completion_times.push_back(diff.count());
            tasks[taskId].actual_effort = static_cast<int>(diff.count());
            task_start_times.erase(taskId);
        }
        completed_task_ids.push_back(taskId); // Mark as complete
        if (circuit_state == CircuitState::HALF_OPEN) {
             circuit_state = CircuitState::CLOSED;
             circuit_breaker_failures = 0;
        }
        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Completed));
    }
}

void Scheduler::setSchedule(const Schedule& schedule) {
    std::map<std::string, int> visit;
    std::function<bool(const std::string&)> hasCycle = [&](const std::string& u) {
        visit[u] = 1;
        for (const auto& task : schedule.tasks) {
            if (task.task_id == u) {
                for (const auto& v : task.dependencies) {
                    if (visit[v] == 1) return true;
                    if (visit[v] == 0 && hasCycle(v)) return true;
                }
            }
        }
        visit[u] = 2;
        return false;
    };
    for (const auto& t : schedule.tasks) {
        if (visit[t.task_id] == 0) {
            if (hasCycle(t.task_id)) {
                logEvent("ERROR", "Dependency cycle detected in schedule: " + schedule.name);
                return;
            }
        }
    }
    current_schedule = schedule;
    logEvent("INFO", "Setting new schedule: " + schedule.name);
    std::vector<Task> sorted = getTopologicallySortedTasks(current_schedule.tasks);
    for (const auto& task : sorted) submitTask(task);
}

void Scheduler::saveSchedule(const std::string& filepath) {
    if (!isValidPath(filepath)) {
        logEvent("ERROR", "Invalid path for saveSchedule: " + filepath);
        return;
    }
    std::ofstream file(filepath);
    file << to_json(current_schedule);
    file.close();
    logEvent("INFO", "Schedule saved to " + filepath);
}

void Scheduler::loadSchedule(const std::string& filepath) {
    if (!isValidPath(filepath)) {
        logEvent("ERROR", "Invalid path for loadSchedule: " + filepath);
        return;
    }
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        setSchedule(schedule_from_json(content));
        file.close();
        logEvent("INFO", "Schedule loaded from " + filepath);
    }
}

void Scheduler::removeTask(const std::string& taskId) {
    auto it = std::remove_if(current_schedule.tasks.begin(), current_schedule.tasks.end(),
        [&taskId](const Task& t) { return t.task_id == taskId; });
    current_schedule.tasks.erase(it, current_schedule.tasks.end());
    tasks.erase(taskId);
    pending_tasks.erase(taskId);
    in_progress_task_ids.erase(std::remove(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId), in_progress_task_ids.end());
    paused_task_ids.erase(std::remove(paused_task_ids.begin(), paused_task_ids.end(), taskId), paused_task_ids.end());
    completed_task_ids.erase(std::remove(completed_task_ids.begin(), completed_task_ids.end(), taskId), completed_task_ids.end());
    logEvent("INFO", "Removed task: " + taskId);
}

void Scheduler::pauseTask(const std::string& taskId) {
    if (tasks.find(taskId) != tasks.end()) {
        if (pending_tasks.find(taskId) != pending_tasks.end()) {
             if (std::find(paused_task_ids.begin(), paused_task_ids.end(), taskId) == paused_task_ids.end()) {
                paused_task_ids.push_back(taskId);
                logEvent("INFO", "Paused task: " + taskId);
                publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Paused));
             }
        }
    }
}

void Scheduler::resumeTask(const std::string& taskId) {
    auto it = std::find(paused_task_ids.begin(), paused_task_ids.end(), taskId);
    if (it != paused_task_ids.end()) {
        paused_task_ids.erase(it);
        logEvent("INFO", "Resumed task: " + taskId);
        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Pending));
    }
}

TaskStatus Scheduler::getTaskStatus(const std::string& taskId) const {
    if (std::find(completed_task_ids.begin(), completed_task_ids.end(), taskId) != completed_task_ids.end()) return TaskStatus::Completed;
    if (std::find(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId) != in_progress_task_ids.end()) return TaskStatus::InProgress;
    if (std::find(paused_task_ids.begin(), paused_task_ids.end(), taskId) != paused_task_ids.end()) return TaskStatus::Paused;
    if (pending_tasks.find(taskId) != pending_tasks.end()) return TaskStatus::Pending;
    return TaskStatus::Failed;
}

void Scheduler::addTaskTemplate(const TaskTemplate& tmpl) {
    templates[tmpl.template_id] = tmpl;
}

Task Scheduler::createTaskFromTemplate(const std::string& templateId, const std::string& newTaskId) {
    if (templates.find(templateId) != templates.end()) {
        logEvent("INFO", "Creating task " + newTaskId + " from template " + templateId);
        Task t = templates[templateId].createTask(newTaskId);
        submitTask(t);
        return tasks[newTaskId];
    }
    return Task();
}

Task Scheduler::cloneTask(const std::string& sourceTaskId, const std::string& newTaskId) {
    if (tasks.find(sourceTaskId) != tasks.end()) {
        Task t = tasks[sourceTaskId];
        t.task_id = newTaskId;
        t.sequence_number = 0;
        t.creation_time = "";
        submitTask(t);
        logEvent("INFO", "Cloned task " + sourceTaskId + " to " + newTaskId);
        return tasks[newTaskId];
    }
    return Task();
}

void Scheduler::cancelTask(const std::string& taskId, const std::string& reason) {
    if (tasks.find(taskId) != tasks.end()) {
        cancellation_reasons[taskId] = reason;
        removeTask(taskId);
        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Failed));
    }
}

void Scheduler::createDraftTask(const Task& task) {
    logEvent("INFO", "Creating draft task: " + task.task_id);
    drafts[task.task_id] = task;
}

void Scheduler::submitDraftTask(const std::string& taskId) {
    if (drafts.find(taskId) != drafts.end()) {
        logEvent("INFO", "Submitting draft task: " + taskId);
        submitTask(drafts[taskId]);
        drafts.erase(taskId);
    }
}

void Scheduler::batchCreateTasks(const std::vector<Task>& tasks_to_add) {
    logEvent("INFO", "Batch creating " + std::to_string(tasks_to_add.size()) + " tasks.");
    for (const auto& t : tasks_to_add) submitTask(t);
}

std::string Scheduler::exportToCSV() const {
    std::stringstream ss;
    ss << "task_id,description,priority,status,owner,due_date\n";
    for (const auto& pair : tasks) {
        const Task& t = pair.second;
        auto escape = [](std::string s) {
            size_t pos = 0;
            while ((pos = s.find("\"", pos)) != std::string::npos) {
                s.replace(pos, 1, "\"\"");
                pos += 2;
            }
            return "\"" + s + "\"";
        };
        ss << escape(t.task_id) << "," << escape(t.description) << "," << escape(t.priority) << ","
           << (int)getTaskStatus(t.task_id) << "," << escape(t.owner) << "," << escape(t.due_date) << "\n";
    }
    return ss.str();
}

void Scheduler::archiveTask(const std::string& taskId) {
    if (tasks.find(taskId) != tasks.end()) {
        tasks[taskId].archived = true;
        logEvent("INFO", "Archived task: " + taskId);
    }
}

void Scheduler::restoreTask(const std::string& taskId) {
    if (tasks.find(taskId) != tasks.end()) tasks[taskId].archived = false;
}

void Scheduler::agePriorities() {
    logEvent("INFO", "Aging task priorities...");
    std::vector<std::string> tids;
    for (const auto& tid : pending_tasks) tids.push_back(tid);
    pending_tasks.clear();
    for (const auto& taskId : tids) {
        Task& t = tasks.at(taskId);
        if (t.priority == "low") t.priority = "medium";
        else if (t.priority == "medium") t.priority = "high";
        pending_tasks.insert(taskId);
    }
}

std::vector<Task> Scheduler::getTopologicallySortedTasks(const std::vector<Task>& tasks_to_sort) const {
    std::map<std::string, Task> task_map;
    for (const auto& t : tasks_to_sort) task_map[t.task_id] = t;
    std::vector<Task> sorted;
    std::map<std::string, int> visit;
    std::function<void(const std::string&)> visit_node = [&](const std::string& id) {
        if (visit[id] == 1) {
            logEvent("ERROR", "Cycle detected in topological sort at task " + id);
            return;
        }
        if (visit[id] == 0) {
            visit[id] = 1;
            if (task_map.count(id)) {
                for (const auto& dep : task_map[id].dependencies) visit_node(dep);
                sorted.push_back(task_map[id]);
            }
            visit[id] = 2;
        }
    };
    for (const auto& t : tasks_to_sort) if (visit[t.task_id] == 0) visit_node(t.task_id);
    return sorted;
}

std::string Scheduler::getCachedCalculation(const std::string& key) {
    if (calculation_cache.count(key)) return calculation_cache[key];
    return "";
}

void Scheduler::setCachedCalculation(const std::string& key, const std::string& value) {
    calculation_cache[key] = value;
}

std::vector<Task> Scheduler::searchTasks(const std::string& query) {
    std::vector<Task> results;
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
    for (const auto& pair : tasks) {
        const Task& t = pair.second;
        std::string lower_desc = t.description;
        std::transform(lower_desc.begin(), lower_desc.end(), lower_desc.begin(), ::tolower);
        if (lower_desc.find(lower_query) != std::string::npos || t.task_id.find(lower_query) != std::string::npos) {
            results.push_back(t);
        }
    }
    return results;
}

std::vector<std::string> Scheduler::detectOrphanedDependencies() const {
    std::vector<std::string> orphans;
    for (const auto& pair : tasks) {
        for (const auto& dep : pair.second.dependencies) if (tasks.find(dep) == tasks.end()) orphans.push_back(dep);
    }
    return orphans;
}

void Scheduler::compactArchive() {
    size_t before = current_schedule.tasks.size();
    auto it = std::remove_if(current_schedule.tasks.begin(), current_schedule.tasks.end(), [](const Task& t) { return t.archived; });
    current_schedule.tasks.erase(it, current_schedule.tasks.end());
    size_t after = current_schedule.tasks.size();
    logEvent("INFO", "Compacted archive. Removed " + std::to_string(before - after) + " tasks.");
}

std::string Scheduler::exportToJSON() const { return to_json(current_schedule); }

void Scheduler::importFromJSON(const std::string& json) {
    Schedule schedule = schedule_from_json(json);
    for (const auto& task : schedule.tasks) {
        if (tasks.count(task.task_id)) {
            logEvent("WARNING", "Import duplicate detected: " + task.task_id + ". Skipping.");
            continue;
        }
        if (!validateTask(task)) {
            logEvent("ERROR", "Field-level validation failed for task: " + task.task_id);
            continue;
        }
        submitTask(task);
    }
}

std::string Scheduler::exportToLineDelimitedJSON() const {
    std::stringstream ss;
    for (const auto& t : current_schedule.tasks) ss << to_json(t) << "\n";
    return ss.str();
}

void Scheduler::createBackup(const std::string& backupPath) { saveSchedule(backupPath); }

void Scheduler::restoreBackup(const std::string& backupPath) { loadSchedule(backupPath); }

void Scheduler::pruneBackups(const std::string& directory, int maxBackups) {
    std::vector<std::filesystem::path> backups;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) if (entry.is_regular_file()) backups.push_back(entry.path());
    if (backups.size() > (size_t)maxBackups) {
        std::sort(backups.begin(), backups.end(), [](const auto& a, const auto& b) { return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b); });
        for (size_t i = 0; i < backups.size() - maxBackups; ++i) std::filesystem::remove(backups[i]);
    }
}

void Scheduler::runMigration(const std::string& targetVersion) {
    logEvent("INFO", "Migrating to version " + targetVersion);
    std::ofstream v_file("schema_version.txt");
    v_file << targetVersion;
    v_file.close();
}

void Scheduler::rollbackMigration(const std::string& rollbackFile) {
    logEvent("INFO", "Rolling back migration using " + rollbackFile);
    std::ifstream r_file(rollbackFile);
    std::string prev_version;
    if (r_file >> prev_version) runMigration(prev_version);
}

void Scheduler::shutdown() {
    logEvent("INFO", "Scheduler shutting down gracefully...");
    saveSchedule("shutdown_state.json");
    logEvent("INFO", "State saved to shutdown_state.json");
}

void Scheduler::logEvent(const std::string& level, const std::string& message) const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "{\"timestamp\": \"" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
              << "\", \"level\": \"" << level << "\", \"message\": \"" << message << "\"}" << std::endl;
}

int Scheduler::getCompletedTaskCount() const { return completed_task_ids.size(); }
int Scheduler::getFailedTaskCount() const { return cancellation_reasons.size(); }
double Scheduler::getRollingAverageCompletionTime() const {
    if (completion_times.empty()) return 0.0;
    double sum = 0;
    for (double t : completion_times) sum += t;
    return sum / completion_times.size();
}

bool Scheduler::validateTask(const Task& task) const {
    if (task.task_id.empty()) { logEvent("ERROR", "Validation failed: task_id is empty."); return false; }
    if (task.description.empty()) { logEvent("ERROR", "Validation failed: description is empty."); return false; }
    return true;
}

std::string Scheduler::renderHumanReadableTimestamp(const std::string& utc_timestamp) const {
    if (utc_timestamp.empty()) return "N/A";
    size_t pos = utc_timestamp.find(" UTC");
    if (pos != std::string::npos) return utc_timestamp.substr(0, pos);
    return utc_timestamp;
}

void Scheduler::setRetryLimit(int limit) { retry_limit = limit; }

void Scheduler::handleTaskFailure(const std::string& taskId) {
    retry_counts[taskId]++;
    circuit_breaker_failures++;
    if (circuit_breaker_failures >= circuit_breaker_threshold) {
        circuit_state = CircuitState::OPEN;
        last_circuit_failure = std::chrono::steady_clock::now();
        logEvent("ERROR", "Circuit breaker OPENED due to multiple failures.");
    }
    if (retry_counts[taskId] < retry_limit) {
        auto it = std::find(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId);
        if (it != in_progress_task_ids.end()) in_progress_task_ids.erase(it);
        int backoff_sec = (1 << retry_counts[taskId]);
        logEvent("INFO", "Task " + taskId + " failed. Retrying in " + std::to_string(backoff_sec) + "s.");
        pending_tasks.insert(taskId);
        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Pending));
    } else {
        logEvent("ERROR", "Task " + taskId + " reached max retries. Moving to dead-letter storage.");
        cancelTask(taskId, "Max retries reached");
        std::filesystem::create_directories("./queue/dead_letter");
        std::string filename = "./queue/dead_letter/" + taskId + ".json";
        std::ofstream o(filename);
        o << to_json(tasks[taskId]) << std::endl;
    }
}

bool Scheduler::isCircuitBroken() const {
    if (circuit_state == CircuitState::OPEN) {
        auto now = std::chrono::steady_clock::now();
        if (now - last_circuit_failure > std::chrono::seconds(30)) {
             const_cast<Scheduler*>(this)->circuit_state = CircuitState::HALF_OPEN;
             return false;
        }
        return true;
    }
    return false;
}

void Scheduler::resetCircuitBreaker() { circuit_breaker_failures = 0; circuit_state = CircuitState::CLOSED; }

// --- Coordinator Implementation ---

Coordinator::Coordinator(Project p, const std::string& queue_dir)
    : project(std::move(p)), scheduler(event_publisher), agent_manager(event_publisher) {
    pending_dir = std::filesystem::path(queue_dir) / "pending";
    in_progress_dir = std::filesystem::path(queue_dir) / "in_progress";
    completed_dir = std::filesystem::path(queue_dir) / "completed";
    failed_dir = std::filesystem::path(queue_dir) / "failed";
    std::filesystem::create_directories(pending_dir);
    std::filesystem::create_directories(in_progress_dir);
    std::filesystem::create_directories(completed_dir);
    std::filesystem::create_directories(failed_dir);
}

void Coordinator::registerAgent(const Agent& agent) { agent_manager.registerAgent(agent); }

void Coordinator::processPendingTasks() {
    ModelBackend model_backend;
    for (const auto& entry : std::filesystem::directory_iterator(pending_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            auto filename = entry.path().filename();
            std::ifstream task_file(entry.path());
            std::string content((std::istreambuf_iterator<char>(task_file)), std::istreambuf_iterator<char>());
            Task task = from_json(content);
            task_file.close();

            auto in_progress_path = in_progress_dir / filename;
            std::filesystem::rename(entry.path(), in_progress_path);
            scheduler.submitTask(task);

            std::string output = "Task processed successfully.";
            if (model_backend.is_available()) {
                output = model_backend.run_model(task.description);
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            scheduler.markTaskAsCompleted(task.task_id);
            auto completed_path = completed_dir / filename;
            std::ofstream result_file(completed_path);
            result_file << "{\"task_id\": \"" << task.task_id << "\", \"status\": \"completed\", \"output\": \"" << output << "\"}" << std::endl;
            result_file.close();
            std::filesystem::remove(in_progress_path);
        }
    }
}

void Coordinator::run() {

    int total_tasks = 0;
    for (const auto& workflow : project.workflows) for (const auto& task : workflow.tasks) { scheduler.submitTask(task); total_tasks++; }
    std::map<std::string, std::chrono::steady_clock::time_point> task_finish_times;
    std::map<std::string, std::string> agent_assignments;
    while (scheduler.getCompletedTaskIds().size() < total_tasks) {
        auto now = std::chrono::steady_clock::now();
        for (auto it = task_finish_times.begin(); it != task_finish_times.end(); ) {
            if (now >= it->second) {
                const std::string& task_id = it->first;
                scheduler.markTaskAsCompleted(task_id);
                for (auto const& [agent_id, assigned_task_id] : agent_assignments) {
                    if (assigned_task_id == task_id) {
                        agent_manager.setAgentState(agent_id, AgentState::IDLE);

                        agent_assignments.erase(agent_id);
                        break;
                    }
                }

                it = task_finish_times.erase(it);
            } else ++it;
        }
        Agent* idle_agent = agent_manager.getIdleAgent();
        if (idle_agent) {
            Task* new_task = scheduler.getNextAvailableTask();
            if (new_task) {
                agent_manager.setAgentState(idle_agent->id, AgentState::BUSY);
                agent_assignments[idle_agent->id] = new_task->task_id;
                task_finish_times[new_task->task_id] = std::chrono::steady_clock::now() + std::chrono::seconds(new_task->max_runtime_sec);

            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}
