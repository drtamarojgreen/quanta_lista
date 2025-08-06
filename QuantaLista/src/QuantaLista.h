#ifndef QUANTALISTA_H
#define QUANTALISTA_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

// Enum for Agent State as defined in the development plan
enum class AgentState {
    IDLE,
    BUSY,
    ERROR
};

// Represents a single task in the system
class Task {
public:
    std::string id;
    std::string name;
    std::string description;
    std::string status; // e.g., "Pending", "In Progress", "Completed"
    std::vector<std::string> dependencies; // List of Task IDs this task depends on
    std::chrono::system_clock::time_point createdAt;

    // Constructor for easy initialization
    Task(std::string t_id, std::string t_name, std::string t_desc, std::string t_status, const std::vector<std::string>& t_deps)
        : id(t_id), name(t_name), description(t_desc), status(t_status), dependencies(t_deps), createdAt(std::chrono::system_clock::now()) {}
};

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

// Represents a collection of tasks that form a logical process
class Workflow {
public:
    std::string id;
    std::string name;
    std::vector<Task> tasks;

    // Constructor
    Workflow(std::string w_id, std::string w_name) : id(w_id), name(w_name) {}

    void addTask(const Task& task) {
        tasks.push_back(task);
    }
};

// Represents the highest-level container for a project
class Project {
public:
    std::string id;
    std::string name;
    std::vector<Workflow> workflows;
    std::vector<Agent> agents;

    // Constructor
    Project(std::string p_id, std::string p_name) : id(p_id), name(p_name) {}

    void addWorkflow(const Workflow& workflow) {
        workflows.push_back(workflow);
    }

    void addAgent(const Agent& agent) {
        agents.push_back(agent);
    }
};

// Forward declaration for the Task class inside the Scheduler context if needed, though it's already defined.
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <algorithm> // Required for std::remove

class Scheduler {
public:
    // Submits all tasks from a workflow to the scheduler.
    void submitWorkflow(const Workflow& workflow);

    // Gets the next task that has its dependencies met.
    Task* getNextAvailableTask();

    // Updates the status of a task.
    void updateTaskStatus(const std::string& taskId, const std::string& newStatus);

private:
    // A map of all tasks known to the scheduler, keyed by task ID for efficient lookup.
    std::map<std::string, Task> tasks;

    // A queue of IDs for tasks that are pending execution.
    std::deque<std::string> pending_task_ids;

    // Checks if all dependencies for a given task are completed.
    bool areDependenciesMet(const Task& task);
};

class AgentManager {
public:
    // Registers a new agent with the manager.
    void registerAgent(const Agent& agent);

    // Finds and returns a pointer to an available (idle) agent.
    Agent* getIdleAgent();

    // Updates the state of a specific agent.
    void setAgentState(const std::string& agentId, AgentState newState);

private:
    // A map of all agents known to the system, keyed by agent ID.
    std::map<std::string, Agent> agents;
};

// The Coordinator class acts as the central engine, managing the scheduler and agents.
class Coordinator {
public:
    // Initializes the coordinator and its components.
    Coordinator();

    // Configures the coordinator with agents and workflows from a project.
    void setupProject(const Project& project);

    // Runs the main simulation loop until all tasks are completed.
    void run();

private:
    Scheduler scheduler;
    AgentManager agent_manager;
    int total_task_count;
};

#endif // QUANTALISTA_H
