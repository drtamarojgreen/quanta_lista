#include "../test_framework.h"
#include "../../src/cli.h"
#include <filesystem>
#include <fstream>
#include <sstream>

void test_no_agents_returns_null_idle() {
    Publisher pub;
    AgentManager am(pub);
    Assert::is_null(am.getIdleAgent(), "Empty manager should return null for idle agent");
}

void test_registered_agent_is_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));

    Agent* agent = am.getIdleAgent();
    Assert::not_null(agent, "Registered agent should be retrievable as idle");
    Assert::equal(agent->id, std::string("a1"), "Agent ID should match");
    Assert::equal(agent->state, AgentState::IDLE, "Freshly registered agent must be IDLE");
}

void test_busy_agent_not_returned_as_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    am.setAgentState("a1", AgentState::BUSY);

    Assert::is_null(am.getIdleAgent(), "BUSY agent must not be returned as idle");
}

void test_agent_returns_to_idle_after_busy() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    am.setAgentState("a1", AgentState::BUSY);
    am.setAgentState("a1", AgentState::IDLE);

    Agent* agent = am.getIdleAgent();
    Assert::not_null(agent, "Agent should be idle again after reset");
    Assert::equal(agent->id, std::string("a1"), "Correct agent returned after state reset");
}

void test_error_agent_not_returned_as_idle() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));
    am.setAgentState("a1", AgentState::ERROR);

    Assert::is_null(am.getIdleAgent(), "ERROR agent must not be returned as idle");
}

void test_idle_agent_selected_from_mixed_pool() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("busy", "Beta"));
    am.registerAgent(Agent("idle", "Gamma"));
    am.setAgentState("busy", AgentState::BUSY);

    Agent* selected = am.getIdleAgent();
    Assert::not_null(selected, "Should find idle agent in pool");
    Assert::equal(selected->id, std::string("idle"), "The idle agent should be selected");
}

void test_get_agent_by_id() {
    Publisher pub;
    AgentManager am(pub);
    am.registerAgent(Agent("a1", "Alpha"));

    const Agent* a = am.getAgent("a1");
    Assert::not_null(a, "getAgent should return agent by ID");
    Assert::equal(a->name, std::string("Alpha"), "Agent name should match");
}

void test_get_nonexistent_agent_returns_null() {
    Publisher pub;
    AgentManager am(pub);

    const Agent* a = am.getAgent("ghost");
    Assert::is_null(a, "Non-existent agent should return null");
}

void test_set_state_on_unknown_agent_is_safe() {
    Publisher pub;
    AgentManager am(pub);
    am.setAgentState("ghost", AgentState::BUSY);
}

void test_empty_scheduler_returns_null() {
    Publisher pub;
    Scheduler scheduler(pub);
    Assert::is_null(scheduler.getNextAvailableTask(), "Empty scheduler should return null");
}

void test_single_task_is_returned() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "task", "high", {}, "c", 1));

    Task* t = scheduler.getNextAvailableTask();
    Assert::not_null(t, "Submitted task should be available");
    Assert::equal(t->task_id, std::string("t1"), "Correct task returned");
}

void test_high_priority_before_low() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("low", "low task", "low", {}, "c", 1));
    scheduler.submitTask(Task("high", "high task", "high", {}, "c", 1));

    Task* first = scheduler.getNextAvailableTask();
    Assert::not_null(first, "First task should not be null");
    Assert::equal(first->task_id, std::string("high"), "High-priority task should be first");
}

void test_priority_order_high_medium_low() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("lo", "low", "low", {}, "c", 1));
    scheduler.submitTask(Task("hi", "high", "high", {}, "c", 1));
    scheduler.submitTask(Task("me", "medium", "medium", {}, "c", 1));

    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("hi"), "1st: high");
    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("me"), "2nd: medium");
    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("lo"), "3rd: low");
}

void test_task_with_unmet_dependency_is_blocked() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("child", "child", "high", {"missing_parent"}, "c", 1));

    Assert::is_null(scheduler.getNextAvailableTask(), "Blocked task should not be scheduled");
}

void test_dependent_task_unblocked_after_parent_completes() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("parent", "parent", "high", {}, "c", 1));
    scheduler.submitTask(Task("child", "child", "high", {"parent"}, "c", 1));

    Task* p = scheduler.getNextAvailableTask();
    Assert::equal(p->task_id, std::string("parent"), "Parent should come first");

    Assert::is_null(scheduler.getNextAvailableTask(), "Child must be blocked before parent done");

    scheduler.markTaskAsCompleted("parent");
    Task* c = scheduler.getNextAvailableTask();
    Assert::not_null(c, "Child should be available after parent completes");
    Assert::equal(c->task_id, std::string("child"), "Correct child task unblocked");
}

void test_three_task_chain_executes_in_order() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "step 1", "high", {}, "c", 1));
    scheduler.submitTask(Task("t2", "step 2", "high", {"t1"}, "c", 1));
    scheduler.submitTask(Task("t3", "step 3", "high", {"t2"}, "c", 1));

    auto advance = [&](const std::string& expected) {
        Task* t = scheduler.getNextAvailableTask();
        Assert::not_null(t, "Expected task: " + expected);
        Assert::equal(t->task_id, expected, "Wrong task in sequential chain");
        scheduler.markTaskAsCompleted(t->task_id);
    };
    advance("t1");
    advance("t2");
    advance("t3");
}

void test_completed_task_ids_accumulate() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "a", "high", {}, "c", 1));
    scheduler.submitTask(Task("t2", "b", "high", {}, "c", 1));

    scheduler.getNextAvailableTask();
    scheduler.markTaskAsCompleted("t1");
    scheduler.getNextAvailableTask();
    scheduler.markTaskAsCompleted("t2");

    Assert::size_equals(scheduler.getCompletedTaskIds(), size_t(2), "Both tasks should be in completed list");
}

void test_json_roundtrip_full_task() {
    Task original("t1", "Do something", "high", {"dep_a", "dep_b"}, "module_x", 60);
    Task restored = from_json(to_json(original));

    Assert::equal(restored.task_id, original.task_id, "task_id");
    Assert::equal(restored.description, original.description, "description");
    Assert::equal(restored.priority, original.priority, "priority");
    Assert::equal(restored.component, original.component, "component");
    Assert::equal(restored.max_runtime_sec, original.max_runtime_sec, "max_runtime_sec");
    Assert::size_equals(restored.dependencies, size_t(2), "dependency count");
    Assert::equal(restored.dependencies[0], std::string("dep_a"), "first dependency");
    Assert::equal(restored.dependencies[1], std::string("dep_b"), "second dependency");
}

void test_json_roundtrip_no_dependencies() {
    Task original("t2", "Solo", "low", {}, "c", 10);
    Task restored = from_json(to_json(original));
    Assert::is_true(restored.dependencies.empty(), "Empty dependencies should round-trip");
}

void test_json_roundtrip_single_dependency() {
    Task original("t3", "Chained", "medium", {"sole"}, "c", 5);
    Task restored = from_json(to_json(original));
    Assert::size_equals(restored.dependencies, size_t(1), "Single dependency count");
    Assert::equal(restored.dependencies[0], std::string("sole"), "Dependency ID");
}

void test_schedule_json_roundtrip() {
    Schedule original("sch1", "Path of Neuroplasticity");
    original.addTask(Task("t1", "Step 1", "high", {}, "c1", 10));
    original.addTask(Task("t2", "Step 2", "medium", {"t1"}, "c2", 20));

    Schedule restored = schedule_from_json(to_json(original));
    Assert::equal(restored.schedule_id, original.schedule_id, "schedule_id");
    Assert::equal(restored.name, original.name, "name");
    Assert::size_equals(restored.tasks, size_t(2), "task count");
    Assert::equal(restored.tasks[0].task_id, std::string("t1"), "first task_id");
    Assert::equal(restored.tasks[1].task_id, std::string("t2"), "second task_id");
}

void test_scheduler_load_save_schedule() {
    Publisher pub;
    Scheduler scheduler(pub);
    Schedule sch("sch2", "Persistent Schedule");
    sch.addTask(Task("p1", "Task 1", "high", {}, "c", 5));
    scheduler.setSchedule(sch);

    std::string path = "test_schedule.json";
    scheduler.saveSchedule(path);
    Assert::is_true(std::filesystem::exists(path), "Schedule file should be created");

    Scheduler scheduler2(pub);
    scheduler2.loadSchedule(path);
    Assert::equal(scheduler2.getSchedule().schedule_id, std::string("sch2"), "Loaded schedule_id");
    Assert::size_equals(scheduler2.getSchedule().tasks, size_t(1), "Loaded task count");

    std::filesystem::remove(path);
}

void test_cli_add_task_creates_json_file() {
    system("rm -rf ./queue");
    char* argv[] = {(char*)"quantalista", (char*)"add", (char*)"task1", (char*)"Test Task", (char*)"high",
                    (char*)"test_comp", (char*)"10", (char*)"dep1,dep2"};
    addTask(8, argv);
    Assert::is_true(std::filesystem::exists("./queue/pending/task1.json"), "task1.json should exist in pending queue");
}

void test_cli_list_tasks_shows_pending_file() {
    system("rm -rf ./queue");
    std::filesystem::create_directories("./queue/pending");
    std::ofstream("./queue/pending/task1.json").close();

    std::stringstream buf;
    std::streambuf* old = std::cout.rdbuf(buf.rdbuf());
    listTasks();
    std::cout.rdbuf(old);

    Assert::is_true(buf.str().find("task1.json") != std::string::npos, "list output should include task1.json");
}

void test_daemon_runs_to_completion_single_agent() {
    system("rm -rf ./queue_unit_daemon");
    Project project("p1", "Unit Daemon Project");
    Workflow wf("wf1", "WF");
    wf.addTask(Task("t1", "analyze", "high", {}, "c", 1));
    wf.addTask(Task("t2", "design", "high", {"t1"}, "c", 1));
    wf.addTask(Task("t3", "implement", "medium", {"t2"}, "c", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_unit_daemon");
    coordinator.registerAgent(Agent("a1", "Solo"));
    coordinator.run();

    const auto& completed = coordinator.getScheduler().getCompletedTaskIds();
    Assert::size_equals(completed, size_t(3), "All 3 tasks should complete");
}

void test_daemon_completes_priority_and_dependency_order() {
    system("rm -rf ./queue_unit_order");
    Project project("p1", "Order Project");
    Workflow wf("wf1", "WF");
    wf.addTask(Task("t1", "low task", "low", {}, "c1", 1));
    wf.addTask(Task("t2", "high task", "high", {}, "c1", 1));
    wf.addTask(Task("t3", "medium task", "medium", {}, "c1", 1));
    wf.addTask(Task("t4", "dependent", "high", {"t2"}, "c1", 1));
    project.addWorkflow(wf);

    Coordinator coordinator(project, "./queue_unit_order");
    coordinator.registerAgent(Agent("a1", "Worker"));
    coordinator.run();

    const auto& c = coordinator.getScheduler().getCompletedTaskIds();
    Assert::size_equals(c, size_t(4), "All 4 tasks should complete");
    Assert::equal(c[0], std::string("t2"), "1st: high-priority independent task");
    Assert::equal(c[1], std::string("t4"), "2nd: high-priority dependent task (after t2)");
    Assert::equal(c[2], std::string("t3"), "3rd: medium-priority task");
    Assert::equal(c[3], std::string("t1"), "4th: low-priority task");
}

int main() {
    std::cout << Color::BOLD << "\n══════════════════════════════════════════\n"
              << "  QuantaLista — Unit Tests\n"
              << "══════════════════════════════════════════\n"
              << Color::RESET;

    UnitTest::section("AgentManager");
    UnitTest::run(test_no_agents_returns_null_idle, "no agents → getIdleAgent() returns null");
    UnitTest::run(test_registered_agent_is_idle, "registered agent is IDLE");
    UnitTest::run(test_busy_agent_not_returned_as_idle, "BUSY agent not returned as idle");
    UnitTest::run(test_agent_returns_to_idle_after_busy, "agent returns to IDLE after BUSY");
    UnitTest::run(test_error_agent_not_returned_as_idle, "ERROR agent not returned as idle");
    UnitTest::run(test_idle_agent_selected_from_mixed_pool, "idle agent selected from mixed pool");
    UnitTest::run(test_get_agent_by_id, "getAgent() returns agent by ID");
    UnitTest::run(test_get_nonexistent_agent_returns_null, "getAgent() returns null for unknown ID");
    UnitTest::run(test_set_state_on_unknown_agent_is_safe, "setAgentState() on unknown ID is safe");

    UnitTest::section("Scheduler");
    UnitTest::run(test_empty_scheduler_returns_null, "empty scheduler returns null");
    UnitTest::run(test_single_task_is_returned, "single submitted task is returned");
    UnitTest::run(test_high_priority_before_low, "high priority scheduled before low");
    UnitTest::run(test_priority_order_high_medium_low, "full priority order: high > medium > low");
    UnitTest::run(test_task_with_unmet_dependency_is_blocked, "task with unmet dependency is blocked");
    UnitTest::run(test_dependent_task_unblocked_after_parent_completes, "dependent task unblocked after parent");
    UnitTest::run(test_three_task_chain_executes_in_order, "three-task chain executes sequentially");
    UnitTest::run(test_completed_task_ids_accumulate, "completed task IDs accumulate correctly");

    UnitTest::section("Task JSON Serialization");
    UnitTest::run(test_json_roundtrip_full_task, "full task round-trips through JSON");
    UnitTest::run(test_json_roundtrip_no_dependencies, "task with no deps round-trips through JSON");
    UnitTest::run(test_json_roundtrip_single_dependency, "task with one dep round-trips through JSON");
    UnitTest::run(test_schedule_json_roundtrip, "schedule round-trips through JSON");

    UnitTest::section("Scheduler Persistence");
    UnitTest::run(test_scheduler_load_save_schedule, "scheduler can load and save schedules");

    UnitTest::section("CLI");
    UnitTest::run(test_cli_add_task_creates_json_file, "add task creates .json file in pending/");
    UnitTest::run(test_cli_list_tasks_shows_pending_file, "list tasks prints pending filenames");

    UnitTest::section("Coordinator / Daemon");
    UnitTest::run(test_daemon_runs_to_completion_single_agent, "daemon completes all tasks (single agent)");
    UnitTest::run(test_daemon_completes_priority_and_dependency_order, "daemon respects priority + dependency order");

    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
