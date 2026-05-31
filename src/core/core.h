#ifndef CORE_H
#define CORE_H

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
#include <set>
#include <functional>

#include "../models/models.h"
#include "../events/events.h"

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

    // Enhancements
    void addTaskTemplate(const TaskTemplate& tmpl);
    Task createTaskFromTemplate(const std::string& templateId, const std::string& newTaskId);
    Task cloneTask(const std::string& sourceTaskId, const std::string& newTaskId);
    void cancelTask(const std::string& taskId, const std::string& reason);
    void createDraftTask(const Task& task);
    void submitDraftTask(const std::string& taskId);
    void batchCreateTasks(const std::vector<Task>& tasks);
    std::string exportToCSV() const;
    void archiveTask(const std::string& taskId);
    void restoreTask(const std::string& taskId);
    void agePriorities();
    std::vector<Task> getTopologicallySortedTasks(const std::vector<Task>& tasks_to_sort) const;
    std::vector<Task> searchTasks(const std::string& query);
    std::string getCachedCalculation(const std::string& key);
    void setCachedCalculation(const std::string& key, const std::string& value);
    std::vector<std::string> detectOrphanedDependencies() const;
    void compactArchive();
    std::string exportToJSON() const;
    void importFromJSON(const std::string& json);
    std::string exportToLineDelimitedJSON() const;
    void createBackup(const std::string& backupPath);
    void restoreBackup(const std::string& backupPath);
    void pruneBackups(const std::string& directory, int maxBackups);
    void runMigration(const std::string& targetVersion);
    void rollbackMigration(const std::string& rollbackFile);
    void shutdown();
    void logEvent(const std::string& level, const std::string& message) const;
    int getCompletedTaskCount() const;
    int getFailedTaskCount() const;
    double getRollingAverageCompletionTime() const;
    bool validateTask(const Task& task) const;
    std::string renderHumanReadableTimestamp(const std::string& utc_timestamp) const;
    void setRetryLimit(int limit);
    void handleTaskFailure(const std::string& taskId);
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
            return 0;
        }
        bool operator()(const std::string& a, const std::string& b) const {
            int priority_a = priority_to_int(tasks_map->at(a).priority);
            int priority_b = priority_to_int(tasks_map->at(b).priority);
            if (priority_a != priority_b) return priority_a > priority_b;
            return a < b;
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

class Coordinator {
public:
    Coordinator(Project p, const std::string& queue_dir);
    void run();
    void registerAgent(const Agent& agent);
    const Scheduler& getScheduler() const { return scheduler; }
    const AgentManager& getAgentManager() const { return agent_manager; }
    Publisher& getEventPublisher() { return event_publisher; }
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

#endif // CORE_H
