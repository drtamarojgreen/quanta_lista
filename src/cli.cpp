#include "QuantaLista.h"
#include "cli.h"
#include "SchedulerUI.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

void addTask(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " add <task_id> <description> <priority> <component> <max_runtime_sec> [dependencies]" << std::endl;
        exit(1);
    }
    Task task;
    task.task_id = argv[2];
    task.description = argv[3];
    task.priority = argv[4];
    task.component = argv[5];
    task.max_runtime_sec = std::stoi(argv[6]);
    if (argc > 7) {
        std::string deps_str = argv[7];
        std::stringstream ss(deps_str);
        std::string dep;
        while (std::getline(ss, dep, ',')) {
            task.dependencies.push_back(dep);
        }
    }

    std::string task_json_str = to_json(task);
    std::filesystem::create_directories("./queue/pending");
    std::string filename = "./queue/pending/" + task.task_id + ".json";
    std::ofstream o(filename);
    o << task_json_str << std::endl;
    std::cout << "Task " << task.task_id << " added to the queue." << std::endl;
}

void listTasks() {
    std::filesystem::create_directories("./queue/pending");
    std::filesystem::create_directories("./queue/in_progress");
    std::filesystem::create_directories("./queue/completed");
    std::filesystem::create_directories("./queue/failed");

    std::cout << "--- Pending Tasks ---" << std::endl;
    if (std::filesystem::exists("./queue/pending")) {
        for (const auto& entry : std::filesystem::directory_iterator("./queue/pending")) {
            std::cout << entry.path().filename() << std::endl;
        }
    }
    std::cout << "--- In Progress Tasks ---" << std::endl;
    if (std::filesystem::exists("./queue/in_progress")) {
        for (const auto& entry : std::filesystem::directory_iterator("./queue/in_progress")) {
            std::cout << entry.path().filename() << std::endl;
        }
    }
    std::cout << "--- Completed Tasks ---" << std::endl;
    if (std::filesystem::exists("./queue/completed")) {
        for (const auto& entry : std::filesystem::directory_iterator("./queue/completed")) {
            std::cout << entry.path().filename() << std::endl;
        }
    }
    std::cout << "--- Failed Tasks ---" << std::endl;
    if (std::filesystem::exists("./queue/failed")) {
        for (const auto& entry : std::filesystem::directory_iterator("./queue/failed")) {
            std::cout << entry.path().filename() << std::endl;
        }
    }
}

void showHelp() {
    std::cout << "QuantaLista CLI - Supported Commands:" << std::endl;
    std::cout << "  add <id> <desc> <prio> <comp> <runtime> [deps] - Add a task" << std::endl;
    std::cout << "  list                                          - List tasks" << std::endl;
    std::cout << "  dashboard                                     - Show dashboard" << std::endl;
    std::cout << "  agent <subcommand>                            - Agent management" << std::endl;
    std::cout << "  workflow <subcommand>                         - Workflow management" << std::endl;
    std::cout << "  backup <subcommand>                           - Backup management" << std::endl;
    std::cout << "  config <subcommand>                           - Configuration management" << std::endl;
    std::cout << "  diagnostics                                   - Run diagnostics" << std::endl;
    std::cout << "  help                                          - Show this help" << std::endl;
}

void handleCommand(int argc, char* argv[]) {
    if (argc < 2) {
        showHelp();
        return;
    }
    std::string cmd = argv[1];
    if (cmd == "add") addTask(argc, argv);
    else if (cmd == "list") listTasks();
    else if (cmd == "dashboard") runDashboard();
    else if (cmd == "agent") {
        if (argc > 3 && std::string(argv[2]) == "register") {
             std::cout << "Registering agent: " << argv[3] << std::endl;
        } else {
             std::cout << "Listing agents..." << std::endl;
        }
    }
    else if (cmd == "workflow") {
        if (argc > 2) std::cout << "Workflow action: " << argv[2] << std::endl;
    }
    else if (cmd == "backup") {
        if (argc > 3 && std::string(argv[2]) == "create") {
             Publisher pub;
             Scheduler s(pub);
             s.createBackup(argv[3]);
             std::cout << "Backup created at " << argv[3] << std::endl;
        }
    }
    else if (cmd == "config") {
        if (argc > 3 && std::string(argv[2]) == "set") {
             std::cout << "Config set: " << argv[3] << std::endl;
        }
    }
    else if (cmd == "diagnostics") {
        std::cout << "Running diagnostics..." << std::endl;
    }
    else if (cmd == "help") showHelp();
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        showHelp();
    }
}

void runDashboard() {
    Publisher pub;
    Scheduler scheduler(pub);
    Greenhouse::UI::SchedulerUI ui;
    std::vector<Agent> agents;
    agents.push_back(Agent("a1", "Agent 1"));

    std::cout << "\033[2J\033[H"; // Clear screen
    std::cout << ui.renderDashboard(scheduler.getSchedule(), agents) << std::endl;
}
