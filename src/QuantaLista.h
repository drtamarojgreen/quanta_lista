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

class Scheduler {
public:
    void submitTask(const Task& task);
    Task* getNextAvailableTask();
    void markTaskAsCompleted(const std::string& taskId);

private:
    std::map<std::string, Task> tasks;
    std::deque<std::string> pending_task_ids;
    std::vector<std::string> completed_task_ids;
    bool areDependenciesMet(const Task& task);
};

class AgentManager {
public:
    void registerAgent(const Agent& agent);
    Agent* getIdleAgent();
    void setAgentState(const std::string& agentId, AgentState newState);

private:
    std::map<std::string, Agent> agents;
};

// The Coordinator class acts as the central engine, managing the scheduler and agents.
class Coordinator {
public:
    Coordinator(const std::string& queue_dir);
    void run(); // This will be the main loop for the daemon

public:
    void processPendingTasks();

private:
    Scheduler scheduler;
    AgentManager agent_manager;
    std::filesystem::path pending_dir;
    std::filesystem::path in_progress_dir;
    std::filesystem::path completed_dir;
    std::filesystem::path failed_dir;
};

#endif // QUANTALISTA_H
