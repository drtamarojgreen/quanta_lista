#include "QuantaLista.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <set>

// Helper function to extract a string value from a JSON string
std::string extract_string(const std::string& json_string, const std::string& key) {
    std::string key_str = "\"" + key + "\": \"";
    size_t start = json_string.find(key_str);
    if (start == std::string::npos) {
        return "";
    }
    start += key_str.length();
    size_t end = json_string.find("\"", start);
    return json_string.substr(start, end - start);
}

// Helper function to extract an int value from a JSON string
int extract_int(const std::string& json_string, const std::string& key) {
    std::string key_str = "\"" + key + "\": ";
    size_t start = json_string.find(key_str);
    if (start == std::string::npos) {
        return 0;
    }
    start += key_str.length();
    size_t end = json_string.find_first_of(",}", start);
    return std::stoi(json_string.substr(start, end - start));
}

// Helper function to extract a string vector from a JSON string
std::vector<std::string> extract_string_vector(const std::string& json_string, const std::string& key) {
    std::vector<std::string> result;
    std::string key_str = "\"" + key + "\": [";
    size_t start = json_string.find(key_str);
    if (start == std::string::npos) {
        return result;
    }
    start += key_str.length();
    size_t end = json_string.find("]", start);
    std::string deps_str = json_string.substr(start, end - start);
    std::stringstream ss(deps_str);
    std::string dep;
    while (std::getline(ss, dep, ',')) {
        size_t first = dep.find("\"");
        size_t last = dep.find_last_of("\"");
        if (first != std::string::npos && last != std::string::npos) {
            result.push_back(dep.substr(first + 1, last - first - 1));
        }
    }
    return result;
}


Task from_json(const std::string& json_string) {
    Task task;
    task.task_id = extract_string(json_string, "task_id");
    task.description = extract_string(json_string, "description");
    task.priority = extract_string(json_string, "priority");
    task.dependencies = extract_string_vector(json_string, "dependencies");
    task.component = extract_string(json_string, "component");
    task.max_runtime_sec = extract_int(json_string, "max_runtime_sec");

    task.date = extract_string(json_string, "date");
    task.time = extract_string(json_string, "time");
    task.platform = extract_string(json_string, "platform");
    task.service = extract_string(json_string, "service");
    // Simple boolean extraction
    task.confirmed = (json_string.find("\"confirmed\": true") != std::string::npos);
    task.overlapping_task_ids = extract_string_vector(json_string, "overlapping_task_ids");
    task.first_name = extract_string(json_string, "first_name");
    task.last_name = extract_string(json_string, "last_name");
    task.contact_info = extract_string(json_string, "contact_info");
    task.anonymous_id = extract_string(json_string, "anonymous_id");
    task.labels = extract_string_vector(json_string, "labels");
    task.due_date = extract_string(json_string, "due_date");
    task.estimated_effort = extract_int(json_string, "estimated_effort");
    task.actual_effort = extract_int(json_string, "actual_effort");
    task.blocked_by_note = extract_string(json_string, "blocked_by_note");
    task.owner = extract_string(json_string, "owner");
    task.watchers = extract_string_vector(json_string, "watchers");
    task.archived = (json_string.find("\"archived\": true") != std::string::npos);
    task.sequence_number = extract_int(json_string, "sequence_number");
    task.cancellation_reason = extract_string(json_string, "cancellation_reason");
    task.creation_time = extract_string(json_string, "creation_time");

    return task;
}

std::string to_json(const Task& task) {
    std::string json_string = "{";
    json_string += "\"task_id\": \"" + task.task_id + "\",";
    json_string += "\"description\": \"" + task.description + "\",";
    json_string += "\"priority\": \"" + task.priority + "\",";
    json_string += "\"dependencies\": [";
    for (size_t i = 0; i < task.dependencies.size(); ++i) {
        json_string += "\"" + task.dependencies[i] + "\"";
        if (i < task.dependencies.size() - 1) {
            json_string += ",";
        }
    }
    json_string += "],";
    json_string += "\"component\": \"" + task.component + "\",";
    json_string += "\"max_runtime_sec\": " + std::to_string(task.max_runtime_sec) + ",";

    json_string += "\"date\": \"" + task.date + "\",";
    json_string += "\"time\": \"" + task.time + "\",";
    json_string += "\"platform\": \"" + task.platform + "\",";
    json_string += "\"service\": \"" + task.service + "\",";
    json_string += "\"confirmed\": " + std::string(task.confirmed ? "true" : "false") + ",";
    json_string += "\"overlapping_task_ids\": [";
    for (size_t i = 0; i < task.overlapping_task_ids.size(); ++i) {
        json_string += "\"" + task.overlapping_task_ids[i] + "\"";
        if (i < task.overlapping_task_ids.size() - 1) {
            json_string += ",";
        }
    }
    json_string += "],";
    json_string += "\"first_name\": \"" + task.first_name + "\",";
    json_string += "\"last_name\": \"" + task.last_name + "\",";
    json_string += "\"contact_info\": \"" + task.contact_info + "\",";
    json_string += "\"anonymous_id\": \"" + task.anonymous_id + "\",";
    json_string += "\"labels\": [";
    for (size_t i = 0; i < task.labels.size(); ++i) {
        json_string += "\"" + task.labels[i] + "\"";
        if (i < task.labels.size() - 1) {
            json_string += ",";
        }
    }
    json_string += "],";
    json_string += "\"due_date\": \"" + task.due_date + "\",";
    json_string += "\"estimated_effort\": " + std::to_string(task.estimated_effort) + ",";
    json_string += "\"actual_effort\": " + std::to_string(task.actual_effort) + ",";
    json_string += "\"blocked_by_note\": \"" + task.blocked_by_note + "\",";
    json_string += "\"owner\": \"" + task.owner + "\",";
    json_string += "\"watchers\": [";
    for (size_t i = 0; i < task.watchers.size(); ++i) {
        json_string += "\"" + task.watchers[i] + "\"";
        if (i < task.watchers.size() - 1) {
            json_string += ",";
        }
    }
    json_string += "],";
    json_string += "\"archived\": " + std::string(task.archived ? "true" : "false") + ",";
    json_string += "\"sequence_number\": " + std::to_string(task.sequence_number) + ",";
    json_string += "\"cancellation_reason\": \"" + task.cancellation_reason + "\",";
    json_string += "\"creation_time\": \"" + task.creation_time + "\"";

    json_string += "}";
    return json_string;
}

std::string to_json(const Schedule& schedule) {
    std::string json_string = "{";
    json_string += "\"name\": \"" + schedule.name + "\",";
    json_string += "\"schedule_id\": \"" + schedule.schedule_id + "\",";
    json_string += "\"tasks\": [";

    std::vector<Task> sorted_tasks = schedule.tasks;
    std::sort(sorted_tasks.begin(), sorted_tasks.end(), [](const Task& a, const Task& b) {
        return a.task_id < b.task_id;
    });

    for (size_t i = 0; i < sorted_tasks.size(); ++i) {
        json_string += to_json(sorted_tasks[i]);
        if (i < sorted_tasks.size() - 1) {
            json_string += ",";
        }
    }
    json_string += "]";
    json_string += "}";
    return json_string;
}

Schedule schedule_from_json(const std::string& json_string) {
    Schedule schedule;
    schedule.schedule_id = extract_string(json_string, "schedule_id");
    schedule.name = extract_string(json_string, "name");

    std::string tasks_key = "\"tasks\": [";
    size_t start = json_string.find(tasks_key);
    if (start != std::string::npos) {
        start += tasks_key.length();
        int brace_count = 0;
        size_t task_start = std::string::npos;
        for (size_t i = start; i < json_string.length(); ++i) {
            if (json_string[i] == '{') {
                if (brace_count == 0) task_start = i;
                brace_count++;
            } else if (json_string[i] == '}') {
                brace_count--;
                if (brace_count == 0 && task_start != std::string::npos) {
                    schedule.tasks.push_back(from_json(json_string.substr(task_start, i - task_start + 1)));
                    task_start = std::string::npos;
                }
            } else if (json_string[i] == ']' && brace_count == 0) {
                break;
            }
        }
    }
    return schedule;
}


// --- AgentManager Class Implementation ---

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
    // Round-robin or simple load balancing
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

void Scheduler::runMigration(const std::string& targetVersion) {
    logEvent("INFO", "Migrating to version " + targetVersion);
    std::ofstream v_file("schema_version.txt");
    v_file << targetVersion;
    v_file.close();
}

void Scheduler::rollbackMigration(const std::string& rollbackFile) {
    logEvent("INFO", "Rolling back migration using " + rollbackFile);
    // Simple mock rollback logic: read from file and update version
    std::ifstream r_file(rollbackFile);
    std::string prev_version;
    if (r_file >> prev_version) {
        runMigration(prev_version);
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
    if (it != agents.end()) {
        return &it->second;
    }
    return nullptr;
}

// --- Scheduler Class Implementation ---

Scheduler::Scheduler(Publisher& pub) : publisher(pub), pending_tasks(TaskComparator{&tasks}) {}

void Scheduler::submitTask(const Task& task) {
    if (tasks.find(task.task_id) == tasks.end()) {
        Task t = task;
        if (t.creation_time.empty()) {
             auto now = std::chrono::system_clock::now();
             auto in_time_t = std::chrono::system_clock::to_time_t(now);
             std::stringstream ss;
             // Enhancement #33: UTC Timestamps
             ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d %H:%M:%S UTC");
             t.creation_time = ss.str();
        }
        if (t.sequence_number == 0) {
            static int next_seq = 1;
            t.sequence_number = next_seq++;
        }
        tasks.emplace(t.task_id, t);
        pending_tasks.insert(t.task_id);

        bool already_in_schedule = false;
        for (const auto& t : current_schedule.tasks) {
            if (t.task_id == task.task_id) {
                already_in_schedule = true;
                break;
            }
        }
        if (!already_in_schedule) {
            current_schedule.addTask(task);
        }

        publisher.publish(TaskCreatedEvent(task.task_id, task.description));
        publisher.publish(TaskStatusChangedEvent(task.task_id, TaskStatus::Pending));
    }
}

bool Scheduler::areDependenciesMet(const Task& task) {
    for (const auto& depId : task.dependencies) {
        if (std::find(completed_task_ids.begin(), completed_task_ids.end(), depId) == completed_task_ids.end()) {
            // Check if dependency even exists
            if (tasks.find(depId) == tasks.end()) {
                logEvent("ERROR", "Dependency " + depId + " for task " + task.task_id + " does not exist.");
            }
            return false;
        }
    }
    return true;
}

Task* Scheduler::getNextAvailableTask() {
    if (isCircuitBroken()) return nullptr;

    for (auto it = pending_tasks.begin(); it != pending_tasks.end(); ++it) {
        Task& task = tasks.at(*it);
        if (std::find(paused_task_ids.begin(), paused_task_ids.end(), *it) != paused_task_ids.end()) {
            continue;
        }
        if (areDependenciesMet(task)) {
            std::string tid = *it;
            in_progress_task_ids.push_back(tid);
            pending_tasks.erase(it);
            task_start_times[tid] = std::chrono::steady_clock::now();
            publisher.publish(TaskStatusChangedEvent(tid, TaskStatus::InProgress));
            return &tasks.at(tid);
        }
    }
    return nullptr;
}

void Scheduler::markTaskAsCompleted(const std::string& taskId) {
    auto it = std::find(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId);
    if (it != in_progress_task_ids.end()) {
        in_progress_task_ids.erase(it);
        completed_task_ids.push_back(taskId);

        if (task_start_times.count(taskId)) {
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = end - task_start_times[taskId];
            completion_times.push_back(diff.count());
            task_start_times.erase(taskId);
        }

        if (circuit_state == CircuitState::HALF_OPEN) {
             circuit_state = CircuitState::CLOSED;
             circuit_breaker_failures = 0;
        }

        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Completed));
    }
}

void Scheduler::setSchedule(const Schedule& schedule) {
    // Enhancement #97: Cycle detection before scheduling
    std::map<std::string, int> visit; // 0: unvisited, 1: visiting, 2: visited
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
    for (const auto& task : sorted) {
        submitTask(task);
    }
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

    // Also remove from internal tracking
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
    if (cancellation_reasons.find(taskId) != cancellation_reasons.end()) return TaskStatus::Failed;
    return TaskStatus::Failed; // Or some unknown/not found status
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
        t.sequence_number = 0; // Will be regenerated in submitTask
        t.creation_time = ""; // Will be regenerated
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

void Scheduler::batchCreateTasks(const std::vector<Task>& tasks) {
    logEvent("INFO", "Batch creating " + std::to_string(tasks.size()) + " tasks.");
    for (const auto& t : tasks) {
        submitTask(t);
    }
}

std::string Scheduler::exportToCSV() const {
    std::stringstream ss;
    ss << "task_id,description,priority,status,owner,due_date\n";
    for (const auto& pair : tasks) {
        const Task& t = pair.second;
        // Simple CSV escaping: replace quotes with double quotes
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
    if (tasks.find(taskId) != tasks.end()) {
        tasks[taskId].archived = false;
    }
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
    std::map<std::string, int> visit; // 0: unvisited, 1: visiting, 2: visited

    std::function<void(const std::string&)> visit_node = [&](const std::string& id) {
        if (visit[id] == 1) {
            logEvent("ERROR", "Cycle detected in topological sort at task " + id);
            return;
        }
        if (visit[id] == 0) {
            visit[id] = 1;
            if (task_map.count(id)) {
                for (const auto& dep : task_map[id].dependencies) {
                    visit_node(dep);
                }
                sorted.push_back(task_map[id]);
            }
            visit[id] = 2;
        }
    };

    for (const auto& t : tasks_to_sort) {
        if (visit[t.task_id] == 0) {
            visit_node(t.task_id);
        }
    }

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
        for (const auto& dep : pair.second.dependencies) {
            if (tasks.find(dep) == tasks.end()) {
                orphans.push_back(dep);
            }
        }
    }
    return orphans;
}

void Scheduler::compactArchive() {
    size_t before = current_schedule.tasks.size();
    auto it = std::remove_if(current_schedule.tasks.begin(), current_schedule.tasks.end(),
        [](const Task& t) { return t.archived; });
    current_schedule.tasks.erase(it, current_schedule.tasks.end());
    size_t after = current_schedule.tasks.size();
    logEvent("INFO", "Compacted archive. Removed " + std::to_string(before - after) + " tasks.");
}

std::string Scheduler::exportToJSON() const {
    return to_json(current_schedule);
}

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
    for (const auto& t : current_schedule.tasks) {
        ss << to_json(t) << "\n";
    }
    return ss.str();
}

void Scheduler::createBackup(const std::string& backupPath) {
    saveSchedule(backupPath);
}

void Scheduler::restoreBackup(const std::string& backupPath) {
    loadSchedule(backupPath);
}

void Scheduler::pruneBackups(const std::string& directory, int maxBackups) {
    std::vector<std::filesystem::path> backups;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) backups.push_back(entry.path());
    }
    if (backups.size() > (size_t)maxBackups) {
        std::sort(backups.begin(), backups.end(), [](const auto& a, const auto& b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });
        for (size_t i = 0; i < backups.size() - maxBackups; ++i) {
            std::filesystem::remove(backups[i]);
        }
    }
}

void Scheduler::shutdown() {
    logEvent("INFO", "Scheduler shutting down gracefully...");
    saveSchedule("shutdown_state.json");
    logEvent("INFO", "State saved to shutdown_state.json");
}

void Scheduler::logEvent(const std::string& level, const std::string& message) const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    // Structured JSON-like output
    std::cout << "{\"timestamp\": \"" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
              << "\", \"level\": \"" << level
              << "\", \"message\": \"" << message << "\"}" << std::endl;
}

int Scheduler::getCompletedTaskCount() const {
    return completed_task_ids.size();
}

int Scheduler::getFailedTaskCount() const {
    return cancellation_reasons.size();
}

double Scheduler::getRollingAverageCompletionTime() const {
    if (completion_times.empty()) return 0.0;
    double sum = 0;
    for (double t : completion_times) sum += t;
    return sum / completion_times.size();
}

bool Scheduler::validateTask(const Task& task) const {
    if (task.task_id.empty()) {
        logEvent("ERROR", "Validation failed: task_id is empty.");
        return false;
    }
    if (task.description.empty()) {
        logEvent("ERROR", "Validation failed: description is empty.");
        return false;
    }
    return true;
}

std::string Scheduler::renderHumanReadableTimestamp(const std::string& utc_timestamp) const {
    if (utc_timestamp.empty()) return "N/A";
    // For now, just strip " UTC" and return, but could be expanded to local time conversion
    size_t pos = utc_timestamp.find(" UTC");
    if (pos != std::string::npos) {
        return utc_timestamp.substr(0, pos);
    }
    return utc_timestamp;
}

bool Scheduler::isValidPath(const std::string& path) {
    if (path.find("..") != std::string::npos) return false;
    return true;
}

void Scheduler::setRetryLimit(int limit) {
    retry_limit = limit;
}

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

        // Exponential backoff: 2^retry_count seconds
        int backoff_sec = (1 << retry_counts[taskId]);
        logEvent("INFO", "Task " + taskId + " failed. Retrying in " + std::to_string(backoff_sec) + "s.");

        // In a real system, we'd schedule this. For now, we move it back to pending but track its retry time.
        pending_tasks.insert(taskId);
        publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Pending));
    } else {
        logEvent("ERROR", "Task " + taskId + " reached max retries. Moving to dead-letter storage.");
        cancelTask(taskId, "Max retries reached");

        // Enhancement #85: Dead-letter storage
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

void Scheduler::resetCircuitBreaker() {
    circuit_breaker_failures = 0;
    circuit_state = CircuitState::CLOSED;
}

#include <filesystem>
#include <fstream>
#include <thread>
#include <ctime>
#include <chrono>
#include <random>

#include <utility>

// --- Coordinator Class Implementation ---

Coordinator::Coordinator(Project p, const std::string& queue_dir)
    : project(std::move(p)),
      scheduler(event_publisher),
      agent_manager(event_publisher) {
    pending_dir = std::filesystem::path(queue_dir) / "pending";
    in_progress_dir = std::filesystem::path(queue_dir) / "in_progress";
    completed_dir = std::filesystem::path(queue_dir) / "completed";
    failed_dir = std::filesystem::path(queue_dir) / "failed";

    std::filesystem::create_directories(pending_dir);
    std::filesystem::create_directories(in_progress_dir);
    std::filesystem::create_directories(completed_dir);
    std::filesystem::create_directories(failed_dir);
}

void Coordinator::registerAgent(const Agent& agent) {
    agent_manager.registerAgent(agent);
}

void Coordinator::processPendingTasks() {
    for (const auto& entry : std::filesystem::directory_iterator(pending_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            auto filename = entry.path().filename();
            // Read and parse the task file
            std::ifstream task_file(entry.path());
            std::string content((std::istreambuf_iterator<char>(task_file)), std::istreambuf_iterator<char>());
            Task task = from_json(content);
            task_file.close();

            std::cout << "Processing task: " << task.task_id << std::endl;

            // Move to in_progress
            auto in_progress_path = in_progress_dir / filename;
            std::filesystem::rename(entry.path(), in_progress_path);

            scheduler.submitTask(task);

            // Simulate task processing
            // In a real scenario, this would be handled by QuantaSensa
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Mark as completed
            scheduler.markTaskAsCompleted(task.task_id);

            // Create result file
            auto completed_path = completed_dir / filename;
            std::ofstream result_file(completed_path);
            std::string result_json_str = "{";
            result_json_str += "\"task_id\": \"" + task.task_id + "\",";
            result_json_str += "\"status\": \"completed\",";
            result_json_str += "\"output\": \"Task processed successfully.\"";
            result_json_str += "}";
            result_file << result_json_str << std::endl;
            result_file.close();

            // Remove from in_progress
            std::filesystem::remove(in_progress_path);

            std::cout << "Task " << task.task_id << " completed." << std::endl;
        }
    }
}

void Coordinator::run() {
    std::cout << "QuantaLista daemon started." << std::endl;

    int total_tasks = 0;
    // Submit all tasks from the project to the scheduler
    for (const auto& workflow : project.workflows) {
        for (const auto& task : workflow.tasks) {
            scheduler.submitTask(task);
            total_tasks++;
        }
    }

    std::map<std::string, std::chrono::steady_clock::time_point> task_finish_times;
    std::map<std::string, std::string> agent_assignments; // agent_id -> task_id

    while (scheduler.getCompletedTaskIds().size() < total_tasks) {
        // 1. Check for completed tasks
        auto now = std::chrono::steady_clock::now();
        for (auto it = task_finish_times.begin(); it != task_finish_times.end(); ) {
            if (now >= it->second) {
                const std::string& task_id = it->first;
                scheduler.markTaskAsCompleted(task_id);

                // Find agent and set to idle
                for (auto const& [agent_id, assigned_task_id] : agent_assignments) {
                    if (assigned_task_id == task_id) {
                        agent_manager.setAgentState(agent_id, AgentState::IDLE);
                        std::cout << "Agent " << agent_id << " is now IDLE." << std::endl;
                        agent_assignments.erase(agent_id);
                        break;
                    }
                }

                std::cout << "Task " << task_id << " completed." << std::endl;
                it = task_finish_times.erase(it);
            } else {
                ++it;
            }
        }

        // 2. Assign new tasks
        Agent* idle_agent = agent_manager.getIdleAgent();
        if (idle_agent) {
            Task* new_task = scheduler.getNextAvailableTask();
            if (new_task) {
                agent_manager.setAgentState(idle_agent->id, AgentState::BUSY);
                agent_assignments[idle_agent->id] = new_task->task_id;
                task_finish_times[new_task->task_id] = std::chrono::steady_clock::now() + std::chrono::seconds(new_task->max_runtime_sec);

                std::cout << "Assigned task " << new_task->task_id << " to agent " << idle_agent->name
                          << ". Expected completion in " << new_task->max_runtime_sec << "s." << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "All tasks from the project have been processed." << std::endl;
}
