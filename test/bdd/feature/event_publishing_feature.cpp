#include "event_publishing_feature.h"
#include "../../test_framework.h"
#include "../step/event_publishing_steps.h"

void feature_event_publishing() {
    Gherkin::feature("Event Publishing")
        .scenario("Registering an agent fires an AgentStateChanged(IDLE) event")
        .given("a Publisher with a subscriber for AgentStateChanged events", []() {})
        .when("a new agent is registered with the AgentManager", []() {})
        .then("exactly one AgentStateChanged event is published", []() { EventPublishingSteps::one_agent_state_changed_on_register(); })
        .and_("the event carries the correct agent ID and IDLE state", []() { EventPublishingSteps::register_event_payload_correct(); })

        .scenario("Setting an agent to the same state does not publish a new event")
        .given("an idle agent and a subscriber", []() {})
        .when("the agent's state is set to IDLE again (no change)", []() {})
        .then("no additional event is published", []() { EventPublishingSteps::no_event_on_same_agent_state(); })

        .scenario("Submitting a task publishes TaskCreated and TaskStatusChanged(Pending) events")
        .given("a Scheduler with subscribers for TaskCreated and TaskStatusChanged", []() {})
        .when("a task is submitted", []() {})
        .then("a TaskCreated event is fired with the task's ID and description", []() { EventPublishingSteps::task_created_event_published(); })
        .and_("a TaskStatusChanged(Pending) event is fired", []() { EventPublishingSteps::task_pending_status_event_published(); })

        .scenario("Completing a task fires a TaskStatusChanged(Completed) event")
        .given("a scheduler with one task that has been moved to InProgress", []() {})
        .when("the task is marked as completed", []() {})
        .then("a TaskStatusChanged(Completed) event is published", []() { EventPublishingSteps::task_completed_status_event_published(); })

        .scenario("Unsubscribing stops further event delivery to that subscriber")
        .given("a subscriber registered for AgentStateChanged events", []() {})
        .when("the subscriber is unsubscribed before an agent is registered", []() {})
        .then("the subscriber receives no events", []() { EventPublishingSteps::unsubscribe_stops_delivery(); })

        .scenario("A full coordinator run fires events in the correct sequence")
        .given("a project with two tasks: T1 (high) and T2 (low, depends on T1)", []() {})
        .and_("one registered agent", []() {})
        .when("the coordinator runs to completion", []() {})
        .then("11 events fire in the expected agent-state and task-status sequence", []() { EventPublishingSteps::coordinator_event_sequence(); })
        .run();
}
