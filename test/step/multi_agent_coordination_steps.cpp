#include "multi_agent_coordination_steps.h"
#include "../test_framework.h"

namespace MultiAgentCoordinationSteps {
void single_task_single_agent_completes() {
    system("rm -rf ./queue_bdd_solo");
    Project project("p", "Solo Project");
    Workflow wf("w", "WF");
    wf.addTask(Task("t1", "sole task", "high", {}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_bdd_solo");
    coordinator.registerAgent(Agent("a1", "Solo"));
    coordinator.run();
    Assert::size_equals(coordinator.getScheduler().getCompletedTaskIds(), size_t(1), "One task should complete");
}

void independent_tasks_complete_hml() {
    system("rm -rf ./queue_bdd_prio");
    Project project("p", "Priority Project");
    Workflow wf("w", "WF");
    wf.addTask(Task("t_lo", "low", "low", {}, "c", 1));
    wf.addTask(Task("t_hi", "high", "high", {}, "c", 1));
    wf.addTask(Task("t_me", "med", "medium", {}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_bdd_prio");
    coordinator.registerAgent(Agent("a1", "Solo"));
    coordinator.run();

    const auto& c = coordinator.getScheduler().getCompletedTaskIds();
    Assert::size_equals(c, size_t(3), "All 3 tasks should complete");
    Assert::equal(c[0], std::string("t_hi"), "1st: high priority");
    Assert::equal(c[1], std::string("t_me"), "2nd: medium priority");
    Assert::equal(c[2], std::string("t_lo"), "3rd: low priority");
}

void chain_completes_all_tasks() {
    system("rm -rf ./queue_bdd_chain");
    Project project("p1", "Chain Project");
    Workflow wf("wf1", "WF");
    wf.addTask(Task("t1", "analyze", "high", {}, "c", 1));
    wf.addTask(Task("t2", "design", "high", {"t1"}, "c", 1));
    wf.addTask(Task("t3", "implement", "medium", {"t2"}, "c", 1));
    wf.addTask(Task("t4", "document", "low", {"t3"}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_bdd_chain");
    coordinator.registerAgent(Agent("a1", "Worker"));
    coordinator.registerAgent(Agent("a2", "Helper"));
    coordinator.run();

    Assert::size_equals(coordinator.getScheduler().getCompletedTaskIds(), size_t(4), "All 4 tasks must complete");
}

void chain_respects_predecessors() {
    system("rm -rf ./queue_bdd_chain2");
    Project project("p1", "Chain Project");
    Workflow wf("wf1", "WF");
    wf.addTask(Task("t1", "analyze", "high", {}, "c", 1));
    wf.addTask(Task("t2", "design", "high", {"t1"}, "c", 1));
    wf.addTask(Task("t3", "implement", "medium", {"t2"}, "c", 1));
    wf.addTask(Task("t4", "document", "low", {"t3"}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_bdd_chain2");
    coordinator.registerAgent(Agent("a1", "Worker"));
    coordinator.run();

    const auto& c = coordinator.getScheduler().getCompletedTaskIds();
    auto pos = [&](const std::string& id) -> int {
        for (int i = 0; i < static_cast<int>(c.size()); ++i) {
            if (c[i] == id) {
                return i;
            }
        }
        return -1;
    };
    Assert::is_true(pos("t1") < pos("t2"), "t1 must complete before t2");
    Assert::is_true(pos("t2") < pos("t3"), "t2 must complete before t3");
    Assert::is_true(pos("t3") < pos("t4"), "t3 must complete before t4");
}

void mixed_priority_dependencies_ordered() {
    system("rm -rf ./queue_bdd_mixed");
    Project project("p1", "Mixed Project");
    Workflow wf("wf1", "WF");
    wf.addTask(Task("t1", "a", "high", {}, "c", 1));
    wf.addTask(Task("t2", "b", "high", {"t1"}, "c", 1));
    wf.addTask(Task("t3", "c", "medium", {}, "c", 1));
    wf.addTask(Task("t4", "d", "low", {"t3"}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_bdd_mixed");
    coordinator.registerAgent(Agent("a1", "Worker"));
    coordinator.run();

    const auto& c = coordinator.getScheduler().getCompletedTaskIds();
    Assert::size_equals(c, size_t(4), "All 4 tasks must complete");
    Assert::equal(c[0], std::string("t1"), "1st: t1 (high, unblocked)");
    Assert::equal(c[1], std::string("t2"), "2nd: t2 (high, after t1)");
    Assert::equal(c[2], std::string("t3"), "3rd: t3 (medium, unblocked)");
    Assert::equal(c[3], std::string("t4"), "4th: t4 (low, after t3)");
}
} // namespace MultiAgentCoordinationSteps
