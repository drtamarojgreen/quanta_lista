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

// Enum for Task Status, used in events
enum class TaskStatus {
    Pending,
    InProgress,
    Completed,
    Failed,
    Paused
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

    // Enhancement 11, 13, 14, 15, 16, 17, 18, 19
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

    // Enhancements 125, 161, 163, 189
    std::string correlation_id;
    bool contains_secrets = false;
    std::string allowed_path;
    size_t payload_size = 0;

    // Default constructor
    Task() = default;

    // Constructor for easy initialization
    Task(std::string id, std::string desc, std::string prio, const std::vector<std::string>& deps, std::string comp, int runtime)
        : task_id(std::move(id)), description(std::move(desc)), priority(std::move(prio)), dependencies(deps), component(std::move(comp)), max_runtime_sec(runtime) {}

};

Task from_json(const std::string& json_string);
std::string to_json(const Task& task);

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

std::string to_json(const Schedule& schedule);
Schedule schedule_from_json(const std::string& json_string);

// Represents a template for creating recurring tasks
class TaskTemplate {
public:
    std::string template_id;
    std::string name;
    std::string description;
    std::string priority;
    std::string component;
    int max_runtime_sec;
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

    // Schedule Management
    void setSchedule(const Schedule& schedule);
    const Schedule& getSchedule() const { return current_schedule; }
    void saveSchedule(const std::string& filepath);
    void loadSchedule(const std::string& filepath);
    void removeTask(const std::string& taskId);

    void pauseTask(const std::string& taskId);
    void resumeTask(const std::string& taskId);
    TaskStatus getTaskStatus(const std::string& taskId) const;

    // Enhancement #1: Task Templates
    void addTaskTemplate(const TaskTemplate& tmpl);
    Task createTaskFromTemplate(const std::string& templateId, const std::string& newTaskId);

    // Enhancement #7: Task Cloning
    Task cloneTask(const std::string& sourceTaskId, const std::string& newTaskId);

    // Enhancement #6: Task Cancellation
    void cancelTask(const std::string& taskId, const std::string& reason);

    // Enhancement #8: Task Draft Status
    void createDraftTask(const Task& task);
    void submitDraftTask(const std::string& taskId);

    // Enhancement #9, 10: Batching
    void batchCreateTasks(const std::vector<Task>& tasks);
    std::string exportToCSV() const;

    // Enhancement #18: Archive
    void archiveTask(const std::string& taskId);
    void restoreTask(const std::string& taskId);

    // Enhancement #3: Aging
    void agePriorities();

    // Enhancement #40: Full-text search
    std::vector<Task> searchTasks(const std::string& query);

    // Enhancement #186, 187: Memoization
    std::string getCachedCalculation(const std::string& key);
    void setCachedCalculation(const std::string& key, const std::string& value);

    // Enhancement #38: Orphaned dependency detection
    std::vector<std::string> detectOrphanedDependencies() const;

    // Enhancement #39: Archive compaction
    void compactArchive();

    // Enhancement #21, 22, 23, 24: Import/Export
    std::string exportToJSON() const;
    void importFromJSON(const std::string& json);
    std::string exportToLineDelimitedJSON() const;

    // Enhancement #26, 27: Backup
    void createBackup(const std::string& backupPath);
    void restoreBackup(const std::string& backupPath);
    void pruneBackups(const std::string& directory, int maxBackups);

    // Enhancement #28, 29, 30, 31: Migrations
    void runMigration(const std::string& targetVersion);
    void rollbackMigration(const std::string& rollbackFile);

    // Enhancement #82: Graceful shutdown
    void shutdown();

    // Enhancement #121: Structured Logging
    void logEvent(const std::string& level, const std::string& message);

    // Enhancement #132: Statistics
    int getCompletedTaskCount() const;
    int getFailedTaskCount() const;
    double getRollingAverageCompletionTime() const;

    // Enhancement #163: Path Traversal Check
    bool isValidPath(const std::string& path);

    // Enhancement #83: Retries
    void setRetryLimit(int limit);
    void handleTaskFailure(const std::string& taskId);

    // Enhancement #84: Circuit Breaker
    bool isCircuitBroken() const;
    void resetCircuitBreaker();

private:
    Schedule current_schedule;
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
    std::map<std::string, Task> drafts;
    std::map<std::string, TaskTemplate> templates;
    std::map<std::string, std::string> cancellation_reasons;
    std::set<std::string, TaskComparator> pending_tasks;
    std::vector<std::string> in_progress_task_ids;
    std::vector<std::string> completed_task_ids;
    std::vector<std::string> paused_task_ids;
    std::map<std::string, int> retry_counts;
    int retry_limit = 3;
    int circuit_breaker_failures = 0;
    const int circuit_breaker_threshold = 5;
    enum class CircuitState { CLOSED, OPEN, HALF_OPEN } circuit_state = CircuitState::CLOSED;
    std::chrono::steady_clock::time_point last_circuit_failure;

    std::map<std::string, std::string> calculation_cache;
    std::vector<double> completion_times;
    std::map<std::string, std::chrono::steady_clock::time_point> task_start_times;
    bool areDependenciesMet(const Task& task);

public:
    const std::vector<std::string>& getCompletedTaskIds() const { return completed_task_ids; }
};

class AgentManager {
public:
    explicit AgentManager(Publisher& pub);
    void registerAgent(const Agent& agent);
    Agent* getIdleAgent();
    Agent* getIdleAgentForTask(const Task& task);
    void setAgentState(const std::string& agentId, AgentState newState);
    const Agent* getAgent(const std::string& agentId) const;
    void updateHeartbeat(const std::string& agentId);
    void checkStaleAgents(int timeout_sec);
    void disableAgent(const std::string& agentId);
    void enableAgent(const std::string& agentId);

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
