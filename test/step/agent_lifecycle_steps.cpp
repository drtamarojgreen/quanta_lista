#include "agent_lifecycle_steps.h"
#include "../test_framework.h"

namespace AgentLifecycleSteps {
void registered_agent_is_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    Agent* agent = am.getIdleAgent();
    Assert::not_null(agent, "Agent should be returned as idle");
    Assert::equal(agent->state, AgentState::IDLE, "State must be IDLE");
}

void busy_agent_not_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("beta", "Beta"));
    am.setAgentState("beta", AgentState::BUSY);
    Assert::is_null(am.getIdleAgent(), "No idle agent expected while Beta is BUSY");
}

void busy_state_is_retrievable() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("beta", "Beta"));
    am.setAgentState("beta", AgentState::BUSY);
    const Agent* a = am.getAgent("beta");
    Assert::not_null(a, "Agent should still be retrievable");
    Assert::equal(a->state, AgentState::BUSY, "State must be BUSY");
}

void busy_agent_returns_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("gamma", "Gamma"));
    am.setAgentState("gamma", AgentState::BUSY);
    am.setAgentState("gamma", AgentState::IDLE);
    Agent* agent = am.getIdleAgent();
    Assert::not_null(agent, "Gamma should be idle again");
    Assert::equal(agent->id, std::string("gamma"), "Correct agent returned");
}

void error_agent_not_selected() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("delta", "Delta"));
    am.setAgentState("delta", AgentState::ERROR);
    Assert::is_null(am.getIdleAgent(), "ERROR agent must not be selected as idle");
}

void idle_agent_preferred_in_pool() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("busy_one", "BusyAgent"));
    am.registerAgent(Agent("idle_one", "IdleAgent"));
    am.setAgentState("busy_one", AgentState::BUSY);
    Agent* selected = am.getIdleAgent();
    Assert::not_null(selected, "An idle agent should be found");
    Assert::equal(selected->id, std::string("idle_one"), "Idle agent should be selected");
}
} // namespace AgentLifecycleSteps
