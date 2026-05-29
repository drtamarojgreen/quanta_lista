#include "json_utils.h"
#include <sstream>
#include <algorithm>
#include <iostream>

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

bool isValidPath(const std::string& path) {
    if (path.find("..") != std::string::npos) return false;
    return true;
}
