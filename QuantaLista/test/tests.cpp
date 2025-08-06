#include "QuantaLista.h"
#include "cli.h"
#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>

// Simple test runner framework
void run_test(void (*test_function)(), const std::string& test_name) {
    try {
        std::cout << "Running test: " << test_name << "..." << std::endl;
        test_function();
        std::cout << "[PASSED] " << test_name << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[FAILED] " << test_name << " with exception: " << e.what() << std::endl << std::endl;
    }
}

// --- Test Cases ---

void test_agent_manager_registration_and_retrieval() {
    AgentManager am;
    assert(am.getIdleAgent() == nullptr); // Should be no agents initially

    am.registerAgent(Agent("agent_1", "TestAgent"));
    Agent* agent = am.getIdleAgent();
    assert(agent != nullptr);
    assert(agent->id == "agent_1");
    assert(agent->state == AgentState::IDLE);
}

void test_agent_manager_state_management() {
    AgentManager am;
    am.registerAgent(Agent("agent_1", "TestAgent"));

    // Set agent to busy and check that no idle agents are found
    am.setAgentState("agent_1", AgentState::BUSY);
    assert(am.getIdleAgent() == nullptr);

    // Set agent back to idle and check that it can be retrieved
    am.setAgentState("agent_1", AgentState::IDLE);
    Agent* agent = am.getIdleAgent();
    assert(agent != nullptr);
    assert(agent->id == "agent_1");
}

void test_add_task() {
    system("rm -rf ./queue");
    char* argv[] = {(char*)"quantalista", (char*)"add", (char*)"task1", (char*)"Test Task", (char*)"high", (char*)"test_comp", (char*)"10", (char*)"dep1,dep2"};
    addTask(8, argv);
    assert(std::filesystem::exists("./queue/pending/task1.json"));
}

void test_list_tasks() {
    system("rm -rf ./queue");
    std::filesystem::create_directories("./queue/pending");
    std::ofstream("./queue/pending/task1.json").close();
    // Redirect cout to a stringstream to capture output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    listTasks();
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    assert(output.find("task1.json") != std::string::npos);
}

void test_daemon_processing() {
    system("rm -rf ./queue");
    char* argv[] = {(char*)"quantalista", (char*)"add", (char*)"task1", (char*)"Test Task", (char*)"high", (char*)"test_comp", (char*)"10", (char*)""};
    addTask(7, argv);

    Coordinator coordinator("./queue");
    coordinator.processPendingTasks();
    assert(std::filesystem::exists("./queue/completed/task1.json"));
}


int main() {
    std::cout << "--- Starting QuantaLista Unit Tests ---" << std::endl << std::endl;

    run_test(test_agent_manager_registration_and_retrieval, "AgentManager: Registration and Retrieval");
    run_test(test_agent_manager_state_management, "AgentManager: State Management");
    run_test(test_add_task, "CLI: Add Task");
    run_test(test_list_tasks, "CLI: List Tasks");
    run_test(test_daemon_processing, "Daemon: Task Processing");

    std::cout << "--- All tests passed successfully! ---" << std::endl;

    return 0;
}
