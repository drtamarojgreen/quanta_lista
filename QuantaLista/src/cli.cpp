#include "QuantaLista.h"
#include "cli.h"
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
