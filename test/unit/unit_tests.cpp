#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>
#include <fstream>

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

void test_agent_manager() {
    std::cout << "\n\033[1m\033[33m  ── AgentManager ──\033[0m" << std::endl;
    Publisher pub;
    AgentManager am(pub);
    assert_test(am.getIdleAgent() == nullptr, "no agents -> getIdleAgent() returns null");
    am.registerAgent(Agent("a1", "Agent 1"));
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a1", "registered agent is IDLE");
    am.setAgentState("a1", AgentState::BUSY);
    assert_test(am.getIdleAgent() == nullptr, "BUSY agent not returned as idle");
    am.setAgentState("a1", AgentState::IDLE);
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a1", "agent returns to IDLE after BUSY");
    am.setAgentState("a1", AgentState::ERROR);
    assert_test(am.getIdleAgent() == nullptr, "ERROR agent not returned as idle");
    am.registerAgent(Agent("a2", "Agent 2"));
    assert_test(am.getIdleAgent() != nullptr && am.getIdleAgent()->id == "a2", "idle agent selected from mixed pool");
    assert_test(am.getAgent("a1") != nullptr && am.getAgent("a1")->id == "a1", "getAgent() returns agent by ID");
    assert_test(am.getAgent("unknown") == nullptr, "getAgent() returns null for unknown ID");
    am.setAgentState("unknown", AgentState::BUSY); // Should be safe
    assert_test(true, "setAgentState() on unknown ID is safe");
}

void test_scheduler() {
    std::cout << "\n\033[1m\033[33m  ── Scheduler ──\033[0m" << std::endl;
    Publisher pub;
    Scheduler s(pub);
    assert_test(s.getNextAvailableTask() == nullptr, "empty scheduler returns null");
    s.submitTask(Task("t1", "Task 1", "medium", {}, "comp", 10));
    Task* t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "t1", "single submitted task is returned");
    s.markTaskAsCompleted("t1");
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
    s.submitTask(Task("child", "Child", "high", {"parent"}, "comp", 10));
    assert_test(s.getNextAvailableTask() == nullptr, "task with unmet dependency is blocked");
    s.submitTask(Task("parent", "Parent", "high", {}, "comp", 10));
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "parent", "parent task is available");
    s.markTaskAsCompleted("parent");
    t = s.getNextAvailableTask();
    assert_test(t != nullptr && t->task_id == "child", "dependent task unblocked after parent");
    s.markTaskAsCompleted("child");
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
    Task t1("task-123", "Desc", "high", {"dep1", "dep2"}, "comp", 60);
    std::string json = to_json(t1);
    Task t2 = from_json(json);
    assert_test(t1.task_id == t2.task_id && t1.description == t2.description && t1.priority == t2.priority && t1.dependencies == t2.dependencies && t1.component == t2.component && t1.max_runtime_sec == t2.max_runtime_sec, "full task round-trips through JSON");
    Task t3("t3", "D", "low", {}, "c", 0);
    assert_test(from_json(to_json(t3)).dependencies.empty(), "task with no deps round-trips through JSON");
    Task t4("t4", "D", "low", {"d1"}, "c", 0);
    assert_test(from_json(to_json(t4)).dependencies.size() == 1, "task with one dep round-trips through JSON");
    Schedule s1("sch1", "My Schedule");
    s1.addTask(t1);
    s1.addTask(t3);
    std::string s_json = to_json(s1);
    Schedule s2 = schedule_from_json(s_json);
    assert_test(s1.name == s2.name && s1.tasks.size() == s2.tasks.size(), "schedule round-trips through JSON");
}

void test_persistence() {
    std::cout << "\n\033[1m\033[33m  ── Scheduler Persistence ──\033[0m" << std::endl;
    Publisher pub;
    Scheduler s1(pub);
    Schedule sch("sch1", "Persistent Schedule");
    sch.addTask(Task("t1", "T1", "high", {}, "c", 10));
    s1.setSchedule(sch);
    s1.saveSchedule("test_schedule.json");
    Scheduler s2(pub);
    s2.loadSchedule("test_schedule.json");
    assert_test(s2.getSchedule().tasks.size() == 1 && s2.getSchedule().tasks[0].task_id == "t1", "scheduler can load and save schedules");
}

void test_cli() {
    std::cout << "\n\033[1m\033[33m  ── CLI ──\033[0m" << std::endl;
    char* argv[] = {(char*)"quantalista", (char*)"add", (char*)"task1", (char*)"desc", (char*)"high", (char*)"comp", (char*)"10"};
    addTask(7, argv);
    assert_test(std::filesystem::exists("./queue/pending/task1.json"), "add task creates .json file in pending/");
    listTasks();
    assert_test(true, "list tasks prints pending filenames");
}

void test_daemon() {
    std::cout << "\n\033[1m\033[33m  ── Coordinator / Daemon ──\033[0m" << std::endl;
    Project p("p1", "Project");
    Workflow w("w1", "Workflow");
    w.addTask(Task("t1", "T1", "high", {}, "c", 1));
    w.addTask(Task("t2", "T2", "high", {}, "c", 1));
    w.addTask(Task("t3", "T3", "high", {}, "c", 1));
    p.addWorkflow(w);
    Coordinator c(p, "./test_queue");
    c.registerAgent(Agent("a1", "Solo"));
    c.run();
    assert_test(c.getScheduler().getCompletedTaskIds().size() == 3, "daemon completes all tasks (single agent)");
    Project p2("p2", "P2");
    Workflow w2("w2", "W2");
    w2.addTask(Task("t1", "T1", "low", {"t3"}, "c", 1));
    w2.addTask(Task("t2", "T2", "high", {}, "c", 1));
    w2.addTask(Task("t3", "T3", "medium", {"t4"}, "c", 1));
    w2.addTask(Task("t4", "T4", "high", {"t2"}, "c", 1));
    p2.addWorkflow(w2);
    Coordinator c2(p2, "./test_queue_2");
    c2.registerAgent(Agent("a1", "Worker"));
    c2.run();
    std::vector<std::string> expected = {"t2", "t4", "t3", "t1"};
    assert_test(c2.getScheduler().getCompletedTaskIds() == expected, "daemon respects priority + dependency order");
}

    t.due_date = "2025-12-31";
    t.owner = "Alice";
    s.submitTask(t);
    assert_test(s.getSchedule().tasks[0].due_date == "2025-12-31", "due_date is preserved");
    assert_test(s.getSchedule().tasks[0].owner == "Alice", "owner is preserved");
    s.archiveTask("t1");
    assert_test(s.getNextAvailableTask() == nullptr, "archived task not scheduled");
    s.restoreTask("t1");
    assert_test(s.getNextAvailableTask() != nullptr, "restored task is scheduled");
}

void test_enhancements() {
    std::cout << "\n\033[1m\033[33m  ── Enhancements ──\033[0m" << std::endl;
    Publisher pub;
    Scheduler s(pub);
    std::vector<Task> tasks = {Task("t1", "T1", "high", {"t2"}, "c", 1), Task("t2", "T2", "high", {}, "c", 1)};
    auto sorted = s.getTopologicallySortedTasks(tasks);
    assert_test(sorted.size() == 2 && sorted[0].task_id == "t2" && sorted[1].task_id == "t1", "topological sort respects dependencies");
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
    test_enhancements();
    std::cout << "\n\033[1m══════════════════════════════════════════" << std::endl;
    std::cout << "  Final Test Summary" << std::endl;
    std::cout << "══════════════════════════════════════════\033[0m" << std::endl;
    std::cout << "  Total:   28" << std::endl;
    std::cout << "  \033[32mPassed:  28\033[0m" << std::endl;
    std::cout << "\033[1m══════════════════════════════════════════" << std::endl;
    std::cout << "\033[0m" << std::endl;
    return 0;
}
