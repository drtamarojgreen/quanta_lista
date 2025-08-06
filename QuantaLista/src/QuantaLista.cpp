#include "QuantaLista.h"
#include <iostream>
#include <vector>

// --- AgentManager Class Implementation ---

void AgentManager::registerAgent(const Agent& agent) {
    agents.emplace(agent.id, agent);
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
        it->second.state = newState;
    }
}

// --- Scheduler Class Implementation ---

void Scheduler::submitWorkflow(const Workflow& workflow) {
    for (const auto& task : workflow.tasks) {
        tasks.emplace(task.id, task);
        if (task.status == "Pending") {
            pending_task_ids.push_back(task.id);
        }
    }
}

bool Scheduler::areDependenciesMet(const Task& task) {
    for (const auto& depId : task.dependencies) {
        auto it = tasks.find(depId);
        if (it == tasks.end() || it->second.status != "Completed") {
            return false;
        }
    }
    return true;
}

Task* Scheduler::getNextAvailableTask() {
    for (const auto& taskId : pending_task_ids) {
        Task& task = tasks.at(taskId);
        if (task.status == "Pending" && areDependenciesMet(task)) {
            return &task;
        }
    }
    return nullptr;
}

void Scheduler::updateTaskStatus(const std::string& taskId, const std::string& newStatus) {
    auto it = tasks.find(taskId);
    if (it != tasks.end()) {
        it->second.status = newStatus;
    }
}

// --- Coordinator Class Implementation ---

Coordinator::Coordinator() : total_task_count(0) {}

void Coordinator::setupProject(const Project& project) {
    std::cout << "Setting up project: " << project.name << std::endl;

    for (const auto& agent : project.agents) {
        agent_manager.registerAgent(agent);
    }
    std::cout << "Registered " << project.agents.size() << " agents." << std::endl;

    for (const auto& workflow : project.workflows) {
        scheduler.submitWorkflow(workflow);
        total_task_count += workflow.tasks.size();
    }
    std::cout << "Submitted " << project.workflows.size() << " workflows with a total of " << total_task_count << " tasks." << std::endl;
}

void Coordinator::run() {
    std::cout << "\n--- Starting Coordinator Simulation Loop ---" << std::endl;
    int completed_tasks_count = 0;
    while (completed_tasks_count < total_task_count) {
        Agent* available_agent = agent_manager.getIdleAgent();
        Task* available_task = scheduler.getNextAvailableTask();

        if (available_agent && available_task) {
            std::cout << "Coordinator assigning task '" << available_task->name << "' to agent '" << available_agent->name << "'." << std::endl;

            agent_manager.setAgentState(available_agent->id, AgentState::BUSY);
            scheduler.updateTaskStatus(available_task->id, "In Progress");
            std::cout << "  - Agent: BUSY, Task: In Progress" << std::endl;

            // Simulate work...
            std::cout << "  - Simulating work for task: " << available_task->name << std::endl;

            scheduler.updateTaskStatus(available_task->id, "Completed");
            agent_manager.setAgentState(available_agent->id, AgentState::IDLE);
            std::cout << "  - Task '" << available_task->name << "' completed." << std::endl;
            std::cout << "  - Agent: IDLE" << std::endl << std::endl;

            completed_tasks_count++;
            continue;
        }

        if (!available_task) {
            std::cout << "No more tasks are ready to be processed. Halting simulation." << std::endl;
            break;
        }

        if (!available_agent) {
             // This case is less likely in a simple synchronous simulation but good to handle.
             // It means a task is ready but no agent is free. We'll just wait for the next cycle.
        }
    }

    std::cout << "--- Coordinator Simulation Finished ---" << std::endl;
    std::cout << "Total tasks completed: " << completed_tasks_count << " out of " << total_task_count << std::endl;
}
