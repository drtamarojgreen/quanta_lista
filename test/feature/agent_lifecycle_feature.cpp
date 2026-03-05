#include "agent_lifecycle_feature.h"
#include "../test_framework.h"
#include "../step/agent_lifecycle_steps.h"

void feature_agent_lifecycle() {
    Gherkin::feature("Agent Lifecycle Management")
        .scenario("An agent begins in IDLE state upon registration")
        .given("an AgentManager with no registered agents", []() {})
        .when("a new agent is registered", []() {})
        .then("the agent is available as idle", []() { AgentLifecycleSteps::registered_agent_is_idle(); })

        .scenario("An IDLE agent transitions to BUSY when assigned work")
        .given("an idle agent named Beta", []() {})
        .when("Beta's state is set to BUSY", []() {})
        .then("Beta is no longer returned as an idle agent", []() { AgentLifecycleSteps::busy_agent_not_idle(); })
        .and_("Beta's state is retrievable as BUSY", []() { AgentLifecycleSteps::busy_state_is_retrievable(); })

        .scenario("A BUSY agent returns to IDLE after completing work")
        .given("agent Gamma is currently BUSY", []() {})
        .when("Gamma's state is set back to IDLE", []() {})
        .then("Gamma is once again returned as an idle agent", []() { AgentLifecycleSteps::busy_agent_returns_idle(); })

        .scenario("An agent in ERROR state is not selected for new work")
        .given("agent Delta is in ERROR state", []() {})
        .when("the coordinator looks for an idle agent", []() {})
        .then("no idle agent is returned", []() { AgentLifecycleSteps::error_agent_not_selected(); })

        .scenario("The idle agent is preferred over a busy one in a pool")
        .given("a pool with one BUSY agent and one IDLE agent", []() {})
        .when("the coordinator requests an idle agent", []() {})
        .then("only the IDLE agent is returned", []() { AgentLifecycleSteps::idle_agent_preferred_in_pool(); })
        .run();
}
