#include "QuantaLista.h"
#include <iostream>
#include <cassert>
#include <string>

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

void test_scheduler_task_with_no_dependencies() {
    Scheduler scheduler;
    Workflow wf("wf_1", "TestWF");
    wf.addTask(Task("task_1", "Standalone Task", "", "Pending", {}));
    scheduler.submitWorkflow(wf);

    Task* task = scheduler.getNextAvailableTask();
    assert(task != nullptr);
    assert(task->id == "task_1");
}

void test_scheduler_task_dependency_not_met() {
    Scheduler scheduler;
    Workflow wf("wf_1", "TestWF");
    wf.addTask(Task("task_1", "Prerequisite Task", "", "Pending", {}));
    wf.addTask(Task("task_2", "Dependent Task", "", "Pending", {"task_1"}));
    scheduler.submitWorkflow(wf);

    // Get the first task, but don't complete it yet
    Task* first_task = scheduler.getNextAvailableTask();
    assert(first_task != nullptr && first_task->id == "task_1");
    scheduler.updateTaskStatus(first_task->id, "In Progress");

    // Try to get the next task, should be nullptr as the dependency is not "Completed"
    Task* second_task = scheduler.getNextAvailableTask();
    assert(second_task == nullptr);
}

void test_scheduler_task_dependency_met() {
    Scheduler scheduler;
    Workflow wf("wf_1", "TestWF");
    wf.addTask(Task("task_1", "Prerequisite Task", "", "Pending", {}));
    wf.addTask(Task("task_2", "Dependent Task", "", "Pending", {"task_1"}));
    scheduler.submitWorkflow(wf);

    // Complete the first task
    scheduler.updateTaskStatus("task_1", "Completed");

    // Now the second task should be available
    Task* task = scheduler.getNextAvailableTask();
    assert(task != nullptr);
    assert(task->id == "task_2");
}

int main() {
    std::cout << "--- Starting QuantaLista Unit Tests ---" << std::endl << std::endl;

    run_test(test_agent_manager_registration_and_retrieval, "AgentManager: Registration and Retrieval");
    run_test(test_agent_manager_state_management, "AgentManager: State Management");
    run_test(test_scheduler_task_with_no_dependencies, "Scheduler: Task with No Dependencies");
    run_test(test_scheduler_task_dependency_not_met, "Scheduler: Task Dependency Not Met");
    run_test(test_scheduler_task_dependency_met, "Scheduler: Task Dependency Met");

    std::cout << "--- All tests passed successfully! ---" << std::endl;

    return 0;
}
