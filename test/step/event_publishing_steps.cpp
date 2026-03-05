#include "event_publishing_steps.h"
#include "../test_framework.h"

namespace EventPublishingSteps {
void one_agent_state_changed_on_register() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::AgentStateChanged, &sub);
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    Assert::size_equals(sub.received_events, size_t(1), "Expected exactly 1 event");
}

void register_event_payload_correct() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::AgentStateChanged, &sub);
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    const auto& e = sub.get<AgentStateChangedEvent>(0);
    Assert::equal(e.agent_id, std::string("a1"), "Event agent_id must match");
    Assert::equal(e.new_state, AgentState::IDLE, "Event state must be IDLE");
}

void no_event_on_same_agent_state() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::AgentStateChanged, &sub);
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    sub.clear();
    am.setAgentState("a1", AgentState::IDLE);
    Assert::size_equals(sub.received_events, size_t(0), "No event expected for same-state transition");
}

void task_created_event_published() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::TaskCreated, &sub);
    pub.subscribe(EventType::TaskStatusChanged, &sub);
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "Do something", "high", {}, "c", 5));
    Assert::equal(sub.count_of<TaskCreatedEvent>(), 1, "Expected 1 TaskCreated event");
    const auto& created = sub.get<TaskCreatedEvent>(0);
    Assert::equal(created.task_id, std::string("t1"), "TaskCreated: task_id");
    Assert::equal(created.description, std::string("Do something"), "TaskCreated: description");
}

void task_pending_status_event_published() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::TaskCreated, &sub);
    pub.subscribe(EventType::TaskStatusChanged, &sub);
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "Do something", "high", {}, "c", 5));
    Assert::equal(sub.count_of<TaskStatusChangedEvent>(), 1, "Expected 1 TaskStatusChanged event");
    const auto& status = sub.get<TaskStatusChangedEvent>(1);
    Assert::equal(status.new_status, TaskStatus::Pending, "Initial status must be Pending");
}

void task_completed_status_event_published() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::TaskStatusChanged, &sub);
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "task", "high", {}, "c", 1));
    scheduler.getNextAvailableTask();
    sub.clear();
    scheduler.markTaskAsCompleted("t1");
    Assert::size_equals(sub.received_events, size_t(1), "Expected exactly 1 event on completion");
    const auto& e = sub.get<TaskStatusChangedEvent>(0);
    Assert::equal(e.new_status, TaskStatus::Completed, "Status must be Completed");
}

void unsubscribe_stops_delivery() {
    Publisher pub;
    MockSubscriber sub;
    pub.subscribe(EventType::AgentStateChanged, &sub);
    pub.unsubscribe(EventType::AgentStateChanged, &sub);
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    Assert::size_equals(sub.received_events, size_t(0), "No events should be received after unsubscribe");
}

void coordinator_event_sequence() {
    Project project("p1", "BDD Project");
    Workflow workflow("wf1", "WF");
    workflow.addTask(Task("t1", "task 1", "high", {}, "c1", 1));
    workflow.addTask(Task("t2", "task 2", "low", {"t1"}, "c1", 1));
    project.addWorkflow(workflow);

    Coordinator coordinator(project, "./queue_bdd_seq");
    MockSubscriber sub;
    Publisher& pub = coordinator.getEventPublisher();
    pub.subscribe(EventType::AgentStateChanged, &sub);
    pub.subscribe(EventType::TaskStatusChanged, &sub);

    coordinator.registerAgent(Agent("bdd_agent", "BDDAgent"));
    coordinator.run();

    Assert::size_equals(sub.received_events, size_t(11), "Expected 11 events total");
    Assert::equal(sub.count_of<AgentStateChangedEvent>(), 5, "Expected 5 AgentStateChanged events");
    Assert::equal(sub.count_of<TaskStatusChangedEvent>(), 6,
                  "Expected 6 TaskStatusChanged events (2 Pending + 2 InProgress + 2 Completed)");

    const auto& e0 = sub.get<AgentStateChangedEvent>(0);
    Assert::equal(e0.agent_id, std::string("bdd_agent"), "Event 0: agent_id");
    Assert::equal(e0.new_state, AgentState::IDLE, "Event 0: IDLE on registration");

    const auto& e1 = sub.get<TaskStatusChangedEvent>(1);
    Assert::equal(e1.new_status, TaskStatus::Pending, "Event 1: t1 Pending");

    const auto& e2 = sub.get<TaskStatusChangedEvent>(2);
    Assert::equal(e2.new_status, TaskStatus::Pending, "Event 2: t2 Pending");

    const auto& e3 = sub.get<TaskStatusChangedEvent>(3);
    Assert::equal(e3.task_id, std::string("t1"), "Event 3: t1 InProgress");
    Assert::equal(e3.new_status, TaskStatus::InProgress, "Event 3: status InProgress");

    const auto& e10 = sub.get<AgentStateChangedEvent>(10);
    Assert::equal(e10.agent_id, std::string("bdd_agent"), "Event 10: final agent_id");
    Assert::equal(e10.new_state, AgentState::IDLE, "Event 10: IDLE after all work");
}
} // namespace EventPublishingSteps
