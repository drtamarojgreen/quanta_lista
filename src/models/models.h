#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

// Enum for Agent State
enum class AgentState {
    IDLE,
    BUSY,
    ERROR
};

// Enum for Task Status
enum class TaskStatus {
    Pending,
    InProgress,
    Completed,
    Failed,
    Paused
};

// Represents a single task in the system
class Task {
public:
    std::string task_id;
    std::string description;
    std::string priority;
    std::vector<std::string> dependencies;
    std::string component;
    int max_runtime_sec = 0;

    // Greenhouse JS Parity Fields
    std::string date;
    std::string time;
    std::string platform;
    std::string service;
    bool confirmed = false;
    std::vector<std::string> overlapping_task_ids;
    std::string first_name;
    std::string last_name;
    std::string contact_info;
    std::string anonymous_id;
    std::vector<std::string> labels;

    // Enhancement fields
    std::string due_date;
    int estimated_effort = 0;
    int actual_effort = 0;
    std::string blocked_by_note;
    std::string owner;
    std::vector<std::string> watchers;
    bool archived = false;
    int sequence_number = 0;
    std::string cancellation_reason;
    std::string creation_time;

    std::string correlation_id;
    bool contains_secrets = false;
    std::string allowed_path;
    size_t payload_size = 0;

    Task() = default;
    Task(std::string id, std::string desc, std::string prio, const std::vector<std::string>& deps, std::string comp, int runtime)
        : task_id(std::move(id)), description(std::move(desc)), priority(std::move(prio)), dependencies(deps), component(std::move(comp)), max_runtime_sec(runtime) {}
};

// Represents a collection of tasks forming a plan
class Schedule {
public:
    std::string schedule_id;
    std::string name;
    std::vector<Task> tasks;

    Schedule() = default;
    Schedule(std::string id, std::string name) : schedule_id(std::move(id)), name(std::move(name)) {}

    void addTask(const Task& task) {
        tasks.push_back(task);
    }
};

// Represents a template for creating recurring tasks
class TaskTemplate {
public:
    std::string template_id;
    std::string name;
    std::string description;
    std::string priority;
    std::string component;
    int max_runtime_sec = 0;
    std::vector<std::string> labels;

    TaskTemplate() = default;
    TaskTemplate(std::string id, std::string name, std::string desc, std::string prio, std::string comp, int runtime)
        : template_id(std::move(id)), name(std::move(name)), description(std::move(desc)), priority(std::move(prio)), component(std::move(comp)), max_runtime_sec(runtime) {}

    Task createTask(const std::string& task_id) const {
        Task t(task_id, description, priority, {}, component, max_runtime_sec);
        t.labels = labels;
        return t;
    }
};

// Represents an agent that can execute tasks
class Agent {
public:
    std::string id;
    std::string name;
    AgentState state;
    std::vector<std::string> capabilities;
    int concurrency_limit = 1;
    int active_tasks = 0;
    std::string last_heartbeat;
    int failure_streak = 0;
    bool disabled = false;

    Agent() = default;
    Agent(std::string a_id, std::string a_name)
        : id(std::move(a_id)), name(std::move(a_name)), state(AgentState::IDLE) {}
};

// Represents a workflow that contains a sequence of tasks
class Workflow {
public:
    std::string id;
    std::string name;
    std::vector<Task> tasks;

    Workflow() = default;
    Workflow(std::string w_id, std::string w_name)
        : id(std::move(w_id)), name(std::move(w_name)) {}

    void addTask(const Task& task) {
        tasks.push_back(task);
    }
};

// Represents a project that contains workflows
class Project {
public:
    std::string id;
    std::string name;
    std::vector<Workflow> workflows;

    Project() = default;
    Project(std::string p_id, std::string p_name)
        : id(std::move(p_id)), name(std::move(p_name)) {}

    void addWorkflow(const Workflow& workflow) {
        workflows.push_back(workflow);
    }
};

#endif // MODELS_H
