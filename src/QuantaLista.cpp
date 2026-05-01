#include "QuantaLista.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

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
    json_string += "\"anonymous_id\": \"" + task.anonymous_id + "\"";

    json_string += "}";
    return json_string;
}

std::string to_json(const Schedule& schedule) {
    std::string json_string = "{";
    json_string += "\"schedule_id\": \"" + schedule.schedule_id + "\",";
    json_string += "\"name\": \"" + schedule.name + "\",";
    json_string += "\"tasks\": [";
    for (size_t i = 0; i < schedule.tasks.size(); ++i) {
        json_string += to_json(schedule.tasks[i]);
        if (i < schedule.tasks.size() - 1) {
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
        if (pair.second.state == AgentState::IDLE) {
            return &pair.second;
        }
    }
    return nullptr;
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
    tasks.emplace(task.task_id, task);
    pending_tasks.insert(task.task_id);
    publisher.publish(TaskCreatedEvent(task.task_id, task.description));
    publisher.publish(TaskStatusChangedEvent(task.task_id, TaskStatus::Pending));
}

bool Scheduler::areDependenciesMet(const Task& task) {
    for (const auto& depId : task.dependencies) {
        if (std::find(completed_task_ids.begin(), completed_task_ids.end(), depId) == completed_task_ids.end()) {
            return false;
        }
    }
    return true;
}

Task* Scheduler::getNextAvailableTask() {
    for (auto it = pending_tasks.begin(); it != pending_tasks.end(); ++it) {
        Task& task = tasks.at(*it);
        if (areDependenciesMet(task)) {
            // Move task from pending to in-progress
            in_progress_task_ids.push_back(*it);
            auto task_ptr = &tasks.at(*it);
            pending_tasks.erase(it);
            publisher.publish(TaskStatusChangedEvent(task_ptr->task_id, TaskStatus::InProgress));
            return task_ptr;
        }
    }
    return nullptr;
}

void Scheduler::markTaskAsCompleted(const std::string& taskId) {
    // Remove from in-progress tasks
    in_progress_task_ids.erase(std::remove(in_progress_task_ids.begin(), in_progress_task_ids.end(), taskId), in_progress_task_ids.end());
    // Add to completed tasks
    completed_task_ids.push_back(taskId);
    publisher.publish(TaskStatusChangedEvent(taskId, TaskStatus::Completed));
}

void Scheduler::setSchedule(const Schedule& schedule) {
    current_schedule = schedule;
    for (const auto& task : current_schedule.tasks) {
        submitTask(task);
    }
}

void Scheduler::saveSchedule(const std::string& filepath) {
    std::ofstream file(filepath);
    file << to_json(current_schedule);
    file.close();
}

void Scheduler::loadSchedule(const std::string& filepath) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        setSchedule(schedule_from_json(content));
        file.close();
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
}

#include <filesystem>
#include <fstream>
#include <thread>
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
