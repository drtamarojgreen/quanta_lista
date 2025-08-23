#include "QuantaLista.h"
#include "cli.h"
#include "pubsub.h"
#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <any>

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

// --- Test Fixtures ---

// Mock Subscriber to capture events for verification
class MockSubscriber : public ISubscriber {
public:
    std::vector<std::any> received_events;

    void onEvent(const Event& event) override {
        switch (event.type) {
            case EventType::TaskCreated:
                received_events.push_back(static_cast<const TaskCreatedEvent&>(event));
                break;
            case EventType::TaskStatusChanged:
                received_events.push_back(static_cast<const TaskStatusChangedEvent&>(event));
                break;
            case EventType::AgentStateChanged:
                received_events.push_back(static_cast<const AgentStateChangedEvent&>(event));
                break;
        }
    }

    void clear() {
        received_events.clear();
    }

    template<typename T>
    int count_events_of_type() {
        int count = 0;
        for (const auto& any_event : received_events) {
            if (any_event.type() == typeid(T)) {
                count++;
            }
        }
        return count;
    }
};


// --- Original Test Cases (Updated) ---

void test_agent_manager_registration_and_retrieval() {
    Publisher pub;
    AgentManager am(pub);
    assert(am.getIdleAgent() == nullptr); // Should be no agents initially

    am.registerAgent(Agent("agent_1", "TestAgent"));
    Agent* agent = am.getIdleAgent();
    assert(agent != nullptr);
    assert(agent->id == "agent_1");
    assert(agent->state == AgentState::IDLE);
}

void test_agent_manager_state_management() {
    Publisher pub;
    AgentManager am(pub);
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
    // This test verifies that the scheduler processes tasks in priority order.
    system("rm -rf ./queue");

    Project project("p1", "Test Project");
    Workflow workflow("wf1", "Test Workflow");
    // Add tasks in non-priority order. t2 (high) should run before t1 (low).
    workflow.addTask(Task("t1", "low priority task", "low", {}, "c1", 1));
    workflow.addTask(Task("t2", "high priority task", "high", {}, "c1", 1));
    workflow.addTask(Task("t3", "medium priority task", "medium", {}, "c1", 1));
    workflow.addTask(Task("t4", "dependent task", "high", {"t2"}, "c1", 1));


    project.addWorkflow(workflow);

    Coordinator coordinator(project, "./queue");
    coordinator.registerAgent(Agent("agent1", "TestAgent1"));

    coordinator.run(); // run will process the tasks and exit

    const auto& completed_tasks = coordinator.getScheduler().getCompletedTaskIds();
    assert(completed_tasks.size() == 4);

    // Check completion order based on priority
    assert(completed_tasks[0] == "t2"); // high
    assert(completed_tasks[1] == "t4"); // high, dependent on t2
    assert(completed_tasks[2] == "t3"); // medium
    assert(completed_tasks[3] == "t1"); // low
}

// --- BDD-Style Tests for Event System ---

void bdd_test_given_subscriber_when_agent_registered_then_event_is_published() {
    // Given
    Publisher pub;
    MockSubscriber mock_sub;
    pub.subscribe(EventType::AgentStateChanged, &mock_sub);
    AgentManager am(pub);

    // When
    am.registerAgent(Agent("agent_bdd_1", "BDDAgent"));

    // Then
    assert(mock_sub.received_events.size() == 1);
    assert(mock_sub.count_events_of_type<AgentStateChangedEvent>() == 1);
    const auto& event = std::any_cast<const AgentStateChangedEvent&>(mock_sub.received_events[0]);
    assert(event.agent_id == "agent_bdd_1");
    assert(event.new_state == AgentState::IDLE);
}

void bdd_test_given_subscriber_when_task_submitted_then_events_are_published() {
    // Given
    Publisher pub;
    MockSubscriber mock_sub;
    pub.subscribe(EventType::TaskCreated, &mock_sub);
    pub.subscribe(EventType::TaskStatusChanged, &mock_sub);
    Scheduler scheduler(pub);
    Task task("bdd_task_1", "BDD Test Task", "high", {}, "bdd_comp", 10);

    // When
    scheduler.submitTask(task);

    // Then
    assert(mock_sub.received_events.size() == 2);
    assert(mock_sub.count_events_of_type<TaskCreatedEvent>() == 1);
    assert(mock_sub.count_events_of_type<TaskStatusChangedEvent>() == 1);

    const auto& e_created = std::any_cast<const TaskCreatedEvent&>(mock_sub.received_events[0]);
    assert(e_created.task_id == "bdd_task_1");
    assert(e_created.description == "BDD Test Task");

    const auto& e_status = std::any_cast<const TaskStatusChangedEvent&>(mock_sub.received_events[1]);
    assert(e_status.task_id == "bdd_task_1");
    assert(e_status.new_status == TaskStatus::Pending);
}

void bdd_test_given_coordinator_when_run_then_correct_events_are_fired() {
    // Given
    Project project("p1", "BDD Project");
    Workflow workflow("wf1", "BDD Workflow");
    workflow.addTask(Task("t1_bdd", "task 1", "high", {}, "c1", 1));
    workflow.addTask(Task("t2_bdd", "task 2", "low", {"t1_bdd"}, "c1", 1));
    project.addWorkflow(workflow);

    Coordinator coordinator(project, "./queue_bdd");
    MockSubscriber mock_sub;
    Publisher& pub = coordinator.getEventPublisher();
    pub.subscribe(EventType::AgentStateChanged, &mock_sub);
    pub.subscribe(EventType::TaskStatusChanged, &mock_sub);

    // When
    coordinator.registerAgent(Agent("bdd_agent", "BDDAgent"));
    coordinator.run();

    // Then
    // Expected events:
    // 1. AgentStateChanged (bdd_agent -> IDLE) on registration
    // 2. TaskStatusChanged (t1_bdd -> InProgress)
    // 3. AgentStateChanged (bdd_agent -> BUSY)
    // 4. TaskStatusChanged (t1_bdd -> Completed)
    // 5. AgentStateChanged (bdd_agent -> IDLE)
    // 6. TaskStatusChanged (t2_bdd -> InProgress)
    // 7. AgentStateChanged (bdd_agent -> BUSY)
    // 8. TaskStatusChanged (t2_bdd -> Completed)
    // 9. AgentStateChanged (bdd_agent -> IDLE)
    assert(mock_sub.received_events.size() == 9);
    assert(mock_sub.count_events_of_type<AgentStateChangedEvent>() == 5);
    assert(mock_sub.count_events_of_type<TaskStatusChangedEvent>() == 4);

    // A few spot checks
    const auto& e1 = std::any_cast<const AgentStateChangedEvent&>(mock_sub.received_events[0]);
    assert(e1.agent_id == "bdd_agent" && e1.new_state == AgentState::IDLE);

    const auto& e2 = std::any_cast<const TaskStatusChangedEvent&>(mock_sub.received_events[1]);
    assert(e2.task_id == "t1_bdd" && e2.new_status == TaskStatus::InProgress);

    const auto& e9 = std::any_cast<const AgentStateChangedEvent&>(mock_sub.received_events[8]);
    assert(e9.agent_id == "bdd_agent" && e9.new_state == AgentState::IDLE);
}


int main() {
    std::cout << "--- Starting QuantaLista Unit Tests ---" << std::endl << std::endl;

    run_test(test_agent_manager_registration_and_retrieval, "AgentManager: Registration and Retrieval");
    run_test(test_agent_manager_state_management, "AgentManager: State Management");
    run_test(test_add_task, "CLI: Add Task");
    run_test(test_list_tasks, "CLI: List Tasks");
    run_test(test_daemon_processing, "Daemon: Task Processing");

    std::cout << "--- Running BDD Tests for Event System ---" << std::endl << std::endl;
    run_test(bdd_test_given_subscriber_when_agent_registered_then_event_is_published, "BDD: Agent Registration Event");
    run_test(bdd_test_given_subscriber_when_task_submitted_then_events_are_published, "BDD: Task Submission Events");
    run_test(bdd_test_given_coordinator_when_run_then_correct_events_are_fired, "BDD: Coordinator Full Run Events");

    std::cout << "--- All tests passed successfully! ---" << std::endl;

    return 0;
}
