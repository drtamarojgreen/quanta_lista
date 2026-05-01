#include "multi_agent_coordination_feature.h"
#include "../../test_framework.h"
#include "../step/multi_agent_coordination_steps.h"

void feature_multi_agent_coordination() {
    Gherkin::feature("Multi-Agent Coordination")
        .scenario("A single task assigned to a single agent completes successfully")
        .given("a project with one task and one registered agent", []() {})
        .when("the coordinator runs", []() {})
        .then("the task appears in the completed task list", []() { MultiAgentCoordinationSteps::single_task_single_agent_completes(); })

        .scenario("Tasks complete in priority order when no dependencies exist")
        .given("a project with high, medium, and low priority independent tasks", []() {})
        .and_("a single agent", []() {})
        .when("the coordinator runs", []() {})
        .then("tasks complete in high, medium, low order", []() { MultiAgentCoordinationSteps::independent_tasks_complete_hml(); })

        .scenario("A four-task dependency chain completes in the correct sequence")
        .given("a project with tasks T1→T2→T3→T4 in a linear dependency chain", []() {})
        .and_("two available agents", []() {})
        .when("the coordinator runs to completion", []() {})
        .then("all four tasks complete", []() { MultiAgentCoordinationSteps::chain_completes_all_tasks(); })
        .and_("each task completes after all its predecessors", []() { MultiAgentCoordinationSteps::chain_respects_predecessors(); })

        .scenario("A mixed-priority project with dependencies respects both constraints")
        .given("tasks T1 (high), T2 (high, depends on T1), T3 (medium), T4 (low, depends on T3)", []() {})
        .and_("a single agent", []() {})
        .when("the coordinator runs", []() {})
        .then("T1 runs first, followed by T2, then T3, then T4", []() { MultiAgentCoordinationSteps::mixed_priority_dependencies_ordered(); })
        .run();
}
