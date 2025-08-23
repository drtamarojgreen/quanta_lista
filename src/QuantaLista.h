#ifndef QUANTALISTA_H
#define QUANTALISTA_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <deque>
#include <map>
#include <algorithm>
#include <filesystem>
#include <fstream>

// Enum for Agent State as defined in the development plan
enum class AgentState {
    IDLE,
    BUSY,
    ERROR
};

// Represents a single task in the system, aligned with the integration plan's JSON structure
class Task {
public:
    std::string task_id;
    std::string description;
    std::string priority;
    std::vector<std::string> dependencies;
    std::string component;
    int max_runtime_sec;

    // Default constructor
    Task() = default;

    // Constructor for easy initialization
    Task(std::string id, std::string desc, std::string prio, const std::vector<std::string>& deps, std::string comp, int runtime)
        : task_id(std::move(id)), description(std::move(desc)), priority(std::move(prio)), dependencies(deps), component(std::move(comp)), max_runtime_sec(runtime) {}

};

Task from_json(const std::string& json_string);
std::string to_json(const Task& task);

// Represents an agent that can execute tasks
class Agent {
public:
    std::string id;
    std::string name;
    AgentState state;

    // Constructor for easy initialization
    Agent(std::string a_id, std::string a_name)
        : id(a_id), name(a_name), state(AgentState::IDLE) {}
};

// Represents a workflow that contains a sequence of tasks
class Workflow {
public:
    std::string id;
    std::string name;
    std::vector<Task> tasks;

    // Default constructor
    Workflow() = default;

    // Constructor for easy initialization
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

    // Default constructor
    Project() = default;

    // Constructor for easy initialization
    Project(std::string p_id, std::string p_name)
        : id(std::move(p_id)), name(std::move(p_name)) {}

    void addWorkflow(const Workflow& workflow) {
        workflows.push_back(workflow);
    }
};

#include <set>
#include <functional>

#include "pubsub.h"

class Publisher; // Forward declaration

class Scheduler {
public:
    Scheduler(Publisher& pub);
    void submitTask(const Task& task);
    Task* getNextAvailableTask();
    void markTaskAsCompleted(const std::string& taskId);

private:
    Publisher& publisher;
    struct TaskComparator {
        const std::map<std::string, Task>* tasks_map;

        int priority_to_int(const std::string& priority) const {
            if (priority == "high") return 2;
            if (priority == "medium") return 1;
            if (priority == "low") return 0;
            return 0; // Default priority
        }

        bool operator()(const std::string& a, const std::string& b) const {
            int priority_a = priority_to_int(tasks_map->at(a).priority);
            int priority_b = priority_to_int(tasks_map->at(b).priority);
            if (priority_a != priority_b) {
                return priority_a > priority_b; // Higher priority value means greater
            }
            return a < b; // Tie-break using task_id for stable ordering
        }
    };

    std::map<std::string, Task> tasks;
    std::set<std::string, TaskComparator> pending_tasks;
    std::vector<std::string> in_progress_task_ids;
    std::vector<std::string> completed_task_ids;
    bool areDependenciesMet(const Task& task);

public:
    const std::vector<std::string>& getCompletedTaskIds() const { return completed_task_ids; }
};

class AgentManager {
public:
    explicit AgentManager(Publisher& pub);
    void registerAgent(const Agent& agent);
    Agent* getIdleAgent();
    void setAgentState(const std::string& agentId, AgentState newState);
    const Agent* getAgent(const std::string& agentId) const;

private:
    Publisher& publisher;
    std::map<std::string, Agent> agents;
};

// The Coordinator class acts as the central engine, managing the scheduler and agents.
class Coordinator {
public:
    Coordinator(Project p, const std::string& queue_dir);
    void run(); // This will be the main loop for the daemon

    void registerAgent(const Agent& agent);

    const Scheduler& getScheduler() const { return scheduler; }
    const AgentManager& getAgentManager() const { return agent_manager; }

    Publisher& getEventPublisher() { return event_publisher; }

public:
    void processPendingTasks();

private:
    Publisher event_publisher;
    Scheduler scheduler;
    AgentManager agent_manager;
    Project project;
    std::filesystem::path pending_dir;
    std::filesystem::path in_progress_dir;
    std::filesystem::path completed_dir;
    std::filesystem::path failed_dir;
};

// --- Event System ---

// Base Event structure for the publisher-subscriber model
enum class EventType {
    TaskCreated,
    TaskStatusChanged,
    AgentStateChanged
};

struct Event {
    virtual ~Event() = default;
    const EventType type;
    const std::chrono::system_clock::time_point timestamp;

protected:
    Event(EventType t) : type(t), timestamp(std::chrono::system_clock::now()) {}
};

// Enum for Task Status, used in events
enum class TaskStatus {
    Pending,
    InProgress,
    Completed,
    Failed
};

// --- Specific Event Structures ---

struct TaskCreatedEvent : public Event {
    const std::string task_id;
    const std::string description;

    TaskCreatedEvent(std::string id, std::string desc)
        : Event(EventType::TaskCreated), task_id(std::move(id)), description(std::move(desc)) {}
};

struct TaskStatusChangedEvent : public Event {
    const std::string task_id;
    const TaskStatus new_status;

    TaskStatusChangedEvent(std::string id, TaskStatus status)
        : Event(EventType::TaskStatusChanged), task_id(std::move(id)), new_status(status) {}
};

struct AgentStateChangedEvent : public Event {
    const std::string agent_id;
    const AgentState new_state;

    AgentStateChangedEvent(std::string id, AgentState state)
        : Event(EventType::AgentStateChanged), agent_id(std::move(id)), new_state(state) {}
};


#endif // QUANTALISTA_H
