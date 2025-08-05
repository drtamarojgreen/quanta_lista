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
struct Task {
    std::string id;
    std::string name;
    std::string description;
    std::string status; // e.g., "Pending", "In Progress", "Completed"
    std::vector<std::string> dependencies; // List of Task IDs this task depends on
    std::chrono::system_clock::time_point createdAt;
};

// Represents an agent that can execute tasks
struct Agent {
    std::string id;
    std::string name;
    AgentState state;
};

// Represents a collection of tasks that form a logical process
struct Workflow {
    std::string id;
    std::string name;
    std::vector<Task> tasks;
};

// Represents the highest-level container for a project
struct Project {
    std::string id;
    std::string name;
    std::vector<Workflow> workflows;
    std::vector<Agent> agents;
};

#endif // QUANTALISTA_H
