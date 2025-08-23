#include "QuantaLista.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

#include "cli.h"

// --- Helper functions for logging enums ---
std::string to_string(TaskStatus status) {
    switch (status) {
        case TaskStatus::Pending: return "Pending";
        case TaskStatus::InProgress: return "InProgress";
        case TaskStatus::Completed: return "Completed";
        case TaskStatus::Failed: return "Failed";
        default: return "Unknown";
    }
}

std::string to_string(AgentState state) {
    switch (state) {
        case AgentState::IDLE: return "IDLE";
        case AgentState::BUSY: return "BUSY";
        case AgentState::ERROR: return "ERROR";
        default: return "Unknown";
    }
}

// --- LoggingSubscriber Class for Demonstration ---
class LoggingSubscriber : public ISubscriber {
public:
    void onEvent(const Event& event) override {
        std::cout << "[EVENT] ";
        switch (event.type) {
            case EventType::TaskCreated: {
                const auto& e = static_cast<const TaskCreatedEvent&>(event);
                std::cout << "TaskCreated: ID=" << e.task_id << ", Desc=\"" << e.description << "\"" << std::endl;
                break;
            }
            case EventType::TaskStatusChanged: {
                const auto& e = static_cast<const TaskStatusChangedEvent&>(event);
                std::cout << "TaskStatusChanged: ID=" << e.task_id << ", NewStatus=" << to_string(e.new_status) << std::endl;
                break;
            }
            case EventType::AgentStateChanged: {
                const auto& e = static_cast<const AgentStateChangedEvent&>(event);
                std::cout << "AgentStateChanged: ID=" << e.agent_id << ", NewState=" << to_string(e.new_state) << std::endl;
                break;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string command = argv[1];
        if (command == "daemon") {
            // Create a sample project as defined in the plan
            Project project("p1", "Sample Project");
            Workflow workflow("wf1", "Sample Workflow");

            // Define tasks with dependencies
            Task task1("task1", "Analyze requirements", "high", {}, "analysis", 10);
            Task task2("task2", "Design architecture", "high", {"task1"}, "design", 20);
            Task task3("task3", "Implement feature", "medium", {"task2"}, "implementation", 30);
            Task task4("task4", "Write documentation", "low", {"task3"}, "documentation", 15);

            workflow.addTask(task1);
            workflow.addTask(task2);
            workflow.addTask(task3);
            workflow.addTask(task4);

            project.addWorkflow(workflow);

            // Initialize the Coordinator with the project
            Coordinator coordinator(project, "./queue");

            // --- Register the Logging Subscriber ---
            LoggingSubscriber logger;
            Publisher& pub = coordinator.getEventPublisher();
            pub.subscribe(EventType::TaskCreated, &logger);
            pub.subscribe(EventType::TaskStatusChanged, &logger);
            pub.subscribe(EventType::AgentStateChanged, &logger);
            std::cout << "Logging subscriber registered." << std::endl;


            // Register a couple of agents
            coordinator.registerAgent(Agent("agent-001", "Researcher"));
            coordinator.registerAgent(Agent("agent-002", "Writer"));

            coordinator.run();
        } else if (command == "add") {
            addTask(argc, argv);
        } else if (command == "list") {
            listTasks();
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <command>" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  daemon      - Run the QuantaLista daemon" << std::endl;
        std::cout << "  add         - Add a new task to the queue" << std::endl;
        std::cout << "  list        - List all tasks in the queue" << std::endl;
    }

    return 0;
}
