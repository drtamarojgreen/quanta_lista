#ifndef MULTI_AGENT_COORDINATION_STEPS_H
#define MULTI_AGENT_COORDINATION_STEPS_H

namespace MultiAgentCoordinationSteps {
void single_task_single_agent_completes();
void independent_tasks_complete_hml();
void chain_completes_all_tasks();
void chain_respects_predecessors();
void mixed_priority_dependencies_ordered();
}

#endif
