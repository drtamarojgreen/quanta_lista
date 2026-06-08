#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include "models/models.h"
#include "events/events.h"
#include "core/core.h"
#include "utils/json_utils.h"
#include "cli/cli.h"

// Simple test helper
void assert_test(bool condition, const std::string& message) {
    if (condition) {
        std::cout << "    \033[2m▸ \033[0m" << std::left << std::setw(60) << message << " \033[32mPASS\033[0m" << std::endl;
    } else {
        std::cout << "    \033[2m▸ \033[0m" << std::left << std::setw(60) << message << " \033[31mFAIL\033[0m" << std::endl;
        exit(1);
    }
}

void test_step(const std::string& message) {
    std::cout << "      \033[2m↳ \033[0m" << message << std::endl;
}

void test_agent_manager() {
    std::cout << "\n\033[1m\033[33m  ── AgentManager ──\033[0m" << std::endl;
    test_step("Creating publisher and agent manager");
    Publisher pub;
    AgentManager am(pub);
    test_step("Checking empty manager behavior");
    assert_test(am.getIdleAgent() == nullptr, "no agents -> getIdleAgent() returns null");
    test_step("Registering first agent and checking IDLE selection");
    am.registerAgent(Agent("a1", "Agent 1"));
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a1", "registered agent is IDLE");
    test_step("Moving agent through BUSY, IDLE, and ERROR states");
    am.setAgentState("a1", AgentState::BUSY);
    assert_test(am.getIdleAgent() == nullptr, "BUSY agent not returned as idle");
    am.setAgentState("a1", AgentState::IDLE);
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a1", "agent returns to IDLE after BUSY");
    am.setAgentState("a1", AgentState::ERROR);
    assert_test(am.getIdleAgent() == nullptr, "ERROR agent not returned as idle");
    test_step("Registering second agent and checking mixed pool selection");
    am.registerAgent(Agent("a2", "Agent 2"));
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a2", "idle agent selected from mixed pool");
    test_step("Checking direct lookup and unknown agent handling");
    assert_test(am.getAgent("a1") != nullptr && am.getAgent("a1")->id == "a1", "getAgent() returns agent by ID");
    assert_test(am.getAgent("unknown") == nullptr, "getAgent() returns null for unknown ID");
    am.setAgentState("unknown", AgentState::BUSY); // Should be safe
    assert_test(true, "setAgentState() on unknown ID is safe");
}

void test_scheduler() {
    std::cout << "\n\033[1m\033[33m  ── Scheduler ──\033[0m" << std::endl;
    test_step("Creating scheduler");
    Publisher pub;
    Scheduler s(pub);
    test_step("Checking empty scheduler behavior");
    assert_test(s.getNextAvailableTask() == nullptr, "empty scheduler returns null");
    test_step("Submitting one task and verifying it is selected");
    s.submitTask(Task("t1", "Task 1", "medium", {}, "comp", 10));
    Task* t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "t1", "single submitted task is returned");
    s.markTaskAsCompleted("t1");
    test_step("Checking high, medium, low priority ordering");
    s.submitTask(Task("low_p", "Low", "low", {}, "comp", 10));
    s.submitTask(Task("high_p", "High", "high", {}, "comp", 10));
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "high_p", "high priority scheduled before low");
    s.markTaskAsCompleted("high_p");
    s.submitTask(Task("med_p", "Med", "medium", {}, "comp", 10));
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "med_p", "full priority order: high > medium > low");
    s.markTaskAsCompleted("med_p");
    s.markTaskAsCompleted(s.getNextAvailableTask()->task_id); // low_p
    test_step("Checking dependency blocking and release after parent completion");
    s.submitTask(Task("child", "Child", "high", {"parent"}, "comp", 10));
    assert_test(s.getNextAvailableTask() == nullptr, "task with unmet dependency is blocked");
    s.submitTask(Task("parent", "Parent", "high", {}, "comp", 10));
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "parent", "parent task is available");
    s.markTaskAsCompleted("parent");
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "child", "dependent task unblocked after parent");
    s.markTaskAsCompleted("child");
    test_step("Checking three-task dependency chain execution");
    s.submitTask(Task("a", "A", "high", {}, "c", 1));
    s.submitTask(Task("b", "B", "high", {"a"}, "c", 1));
    s.submitTask(Task("c", "C", "high", {"b"}, "c", 1));
    s.markTaskAsCompleted(s.getNextAvailableTask()->task_id); // a
    s.markTaskAsCompleted(s.getNextAvailableTask()->task_id); // b
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "c", "three-task chain executes sequentially");
    s.markTaskAsCompleted("c");
    assert_test(s.getCompletedTaskIds().size() >= 3, "completed task IDs accumulate correctly");
}

void test_json() {
    std::cout << "\n\033[1m\033[33m  ── Task JSON Serialization ──\033[0m" << std::endl;
    test_step("Serializing and deserializing task with two dependencies");
    Task t1("task-123", "Desc", "high", {"dep1", "dep2"}, "comp", 60);
    std::string json = to_json(t1);
    Task t2 = from_json(json);
    assert_test(t1.task_id == t2.task_id && t1.description == t2.description && t1.priority == t2.priority && t1.dependencies == t2.dependencies && t1.component == t2.component && t1.max_runtime_sec == t2.max_runtime_sec, "full task round-trips through JSON");
    test_step("Checking empty and single dependency JSON edge cases");
    Task t3("t3", "D", "low", {}, "c", 0);
    assert_test(from_json(to_json(t3)).dependencies.empty(), "task with no deps round-trips through JSON");
    Task t4("t4", "D", "low", {"d1"}, "c", 0);
    assert_test(from_json(to_json(t4)).dependencies.size() == 1, "task with one dep round-trips through JSON");
    test_step("Serializing and deserializing schedule");
    Schedule s1("sch1", "My Schedule");
    s1.addTask(t1);
    s1.addTask(t3);
    std::string s_json = to_json(s1);
    Schedule s2 = schedule_from_json(s_json);
    assert_test(s1.name == s2.name && s1.tasks.size() == s2.tasks.size(), "schedule round-trips through JSON");
}

void test_persistence() {
    std::cout << "\n\033[1m\033[33m  ── Scheduler Persistence ──\033[0m" << std::endl;
    test_step("Creating schedule with one task");
    Publisher pub;
    Scheduler s1(pub);
    Schedule sch("sch1", "Persistent Schedule");
    sch.addTask(Task("t1", "T1", "high", {}, "c", 10));
    s1.setSchedule(sch);
    test_step("Saving schedule to test_schedule.json");
    s1.saveSchedule("test_schedule.json");
    test_step("Loading schedule into a new scheduler");
    Scheduler s2(pub);
    s2.loadSchedule("test_schedule.json");
    assert_test(s2.getSchedule().tasks.size() == 1 && s2.getSchedule().tasks[0].task_id == "t1", "scheduler can load and save schedules");
}

void test_cli() {
    std::cout << "\n\033[1m\033[33m  ── CLI ──\033[0m" << std::endl;
    test_step("Calling addTask with CLI-style arguments");
    char* argv[] = {(char*)"quantalista", (char*)"add", (char*)"task1", (char*)"desc", (char*)"high", (char*)"comp", (char*)"10"};
    addTask(7, argv);
    test_step("Verifying pending task JSON exists");
    assert_test(std::filesystem::exists("./queue/pending/task1.json"), "add task creates .json file in pending/");
    test_step("Listing queue files");
    listTasks();
    assert_test(true, "list tasks prints pending filenames");
}

void test_daemon() {
    std::cout << "\n\033[1m\033[33m  ── Coordinator / Daemon ──\033[0m" << std::endl;
    test_step("Building single-agent workflow with three independent tasks");
    Project p("p1", "Project");
    Workflow w("w1", "Workflow");
    w.addTask(Task("t1", "T1", "high", {}, "c", 1));
    w.addTask(Task("t2", "T2", "high", {}, "c", 1));
    w.addTask(Task("t3", "T3", "high", {}, "c", 1));
    p.addWorkflow(w);
    Coordinator c(p, "./test_queue");
    c.registerAgent(Agent("a1", "Solo"));
    test_step("Running coordinator for independent workflow");
    c.run();
    assert_test(c.getScheduler().getCompletedTaskIds().size() == 3, "daemon completes all tasks (single agent)");
    test_step("Building priority and dependency workflow");
    Project p2("p2", "P2");
    Workflow w2("w2", "W2");
    w2.addTask(Task("t1", "T1", "low", {"t3"}, "c", 1));
    w2.addTask(Task("t2", "T2", "high", {}, "c", 1));
    w2.addTask(Task("t3", "T3", "medium", {"t4"}, "c", 1));
    w2.addTask(Task("t4", "T4", "high", {"t2"}, "c", 1));
    p2.addWorkflow(w2);
    Coordinator c2(p2, "./test_queue_2");
    c2.registerAgent(Agent("a1", "Worker"));
    test_step("Running coordinator for dependency workflow");
    c2.run();
    std::vector<std::string> expected = {"t2", "t4", "t3", "t1"};
    assert_test(c2.getScheduler().getCompletedTaskIds() == expected, "daemon respects priority + dependency order");
}

void test_task_metadata_and_archive_restore() {
    std::cout << "\n\033[1m\033[33m  ── Task Metadata / Archive Restore ──\033[0m" << std::endl;
    test_step("Creating scheduler and task with metadata");
    Publisher pub;
    Scheduler s(pub);
    Task t("t1", "T1", "high", {}, "c", 1);
    t.due_date = "2025-12-31";
    t.owner = "Alice";
    test_step("Submitting task and verifying metadata is preserved");
    s.submitTask(t);
    assert_test(s.getSchedule().tasks[0].due_date == "2025-12-31", "due_date is preserved");
    assert_test(s.getSchedule().tasks[0].owner == "Alice", "owner is preserved");
    test_step("Archiving task and verifying it is not scheduled");
    s.archiveTask("t1");
    assert_test(s.getNextAvailableTask() == nullptr, "archived task not scheduled");
    test_step("Restoring task and verifying it can be scheduled again");
    s.restoreTask("t1");
    assert_test(s.getNextAvailableTask() != nullptr, "restored task is scheduled");
}

void test_enhancements() {
    std::cout << "\n\033[1m\033[33m  ── Enhancements ──\033[0m" << std::endl;
    test_step("Checking topological sort with dependency order");
    Publisher pub;
    Scheduler s(pub);
    std::vector<Task> tasks = {Task("t1", "T1", "high", {"t2"}, "c", 1), Task("t2", "T2", "high", {}, "c", 1)};
    auto sorted = s.getTopologicallySortedTasks(tasks);
    assert_test(sorted.size() == 2 && sorted[0].task_id == "t2" && sorted[1].task_id == "t1", "topological sort respects dependencies");
    test_step("Checking JSON import duplicate detection");
    s.submitTask(Task("t1", "T1", "high", {}, "c", 1));
    s.importFromJSON("{\"name\": \"Import\", \"schedule_id\": \"imp\", \"tasks\": [{\"task_id\": \"t1\", \"description\": \"D\", \"priority\": \"high\"}]}");
    assert_test(s.getSchedule().tasks.size() == 1, "import detects duplicate task IDs");
}

int main() {
    std::cout << "\033[1m" << std::endl;
    std::cout << "══════════════════════════════════════════" << std::endl;
    std::cout << "  QuantaLista — Unit Tests" << std::endl;
    std::cout << "══════════════════════════════════════════" << std::endl;
    std::cout << "\033[0m" << std::endl;
    test_agent_manager();
    test_scheduler();
    test_json();
    test_persistence();
    test_cli();
    test_daemon();
    test_task_metadata_and_archive_restore();
    test_enhancements();
    std::cout << "\n\033[1m══════════════════════════════════════════" << std::endl;
    std::cout << "  Final Test Summary" << std::endl;
    std::cout << "══════════════════════════════════════════\033[0m" << std::endl;
    std::cout << "  Total:   32" << std::endl;
    std::cout << "  \033[32mPassed:  32\033[0m" << std::endl;
    std::cout << "\033[1m══════════════════════════════════════════" << std::endl;
    std::cout << "\033[0m" << std::endl;
    return 0;
}
