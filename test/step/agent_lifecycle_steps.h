#ifndef AGENT_LIFECYCLE_STEPS_H
#define AGENT_LIFECYCLE_STEPS_H

namespace AgentLifecycleSteps {
void registered_agent_is_idle();
void busy_agent_not_idle();
void busy_state_is_retrievable();
void busy_agent_returns_idle();
void error_agent_not_selected();
void idle_agent_preferred_in_pool();
}

#endif
