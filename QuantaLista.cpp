#include "QuantaLista.h"
#include <iostream>

// QuantaLista main application
// PrismQuanta LLM agent monitoring and advisory app

// Helper function to print project details for verification
void print_project_details(const Project& project) {
    std::cout << "Project ID: " << project.id << std::endl;
    std::cout << "Project Name: " << project.name << std::endl;
    std::cout << "---" << std::endl;

    std::cout << "Agents:" << std::endl;
    for (const auto& agent : project.agents) {
        std::cout << "  - Agent ID: " << agent.id << ", Name: " << agent.name << ", State: ";
        switch (agent.state) {
            case AgentState::IDLE: std::cout << "IDLE"; break;
            case AgentState::BUSY: std::cout << "BUSY"; break;
            case AgentState::ERROR: std::cout << "ERROR"; break;
        }
        std::cout << std::endl;
    }
    std::cout << "---" << std::endl;

    std::cout << "Workflows:" << std::endl;
    for (const auto& workflow : project.workflows) {
        std::cout << "  - Workflow ID: " << workflow.id << ", Name: " << workflow.name << std::endl;
        for (const auto& task : workflow.tasks) {
            std::cout << "    - Task ID: " << task.id << ", Name: '" << task.name << "', Status: " << task.status << std::endl;
        }
    }
}

int main() {
    // Create a sample project to demonstrate the data structures
    Project my_project;
    my_project.id = "proj_001";
    my_project.name = "Autonomous Research Agent Development";

    // Create agents
    Agent agent1 = {"agent_01", "Researcher", AgentState::IDLE};
    Agent agent2 = {"agent_02", "Writer", AgentState::IDLE};
    my_project.agents.push_back(agent1);
    my_project.agents.push_back(agent2);

    // Create a workflow with tasks
    Workflow research_workflow;
    research_workflow.id = "wf_01";
    research_workflow.name = "Data Gathering and Analysis";

    Task task1 = {"task_01", "Gather sources", "Find 10 relevant academic papers.", "Pending", {}, std::chrono::system_clock::now()};
    Task task2 = {"task_02", "Summarize papers", "Create a summary for each paper.", "Pending", {"task_01"}, std::chrono::system_clock::now()};
    Task task3 = {"task_03", "Synthesize findings", "Write a literature review.", "Pending", {"task_02"}, std::chrono::system_clock::now()};

    research_workflow.tasks.push_back(task1);
    research_workflow.tasks.push_back(task2);
    research_workflow.tasks.push_back(task3);

    my_project.workflows.push_back(research_workflow);

    // Print the details of the sample project
    print_project_details(my_project);

    return 0;
}
