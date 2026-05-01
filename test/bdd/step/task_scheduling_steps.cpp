#include "task_scheduling_steps.h"
#include "../../test_framework.h"

namespace TaskSchedulingSteps {
void high_before_low() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("low", "low task", "low", {}, "c", 1));
    scheduler.submitTask(Task("high", "high task", "high", {}, "c", 1));
    Task* t = scheduler.getNextAvailableTask();
    Assert::not_null(t, "A task should be returned");
    Assert::equal(t->task_id, std::string("high"), "High-priority task must come first");
}

void priority_order_hml() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("lo", "low", "low", {}, "c", 1));
    scheduler.submitTask(Task("hi", "high", "high", {}, "c", 1));
    scheduler.submitTask(Task("me", "medium", "medium", {}, "c", 1));
    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("hi"), "1st: high");
    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("me"), "2nd: medium");
    Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("lo"), "3rd: low");
}

void unmet_dependency_not_scheduled() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("child", "child", "high", {"missing_parent"}, "c", 1));
    Assert::is_null(scheduler.getNextAvailableTask(), "Blocked task should not be scheduled");
}

void child_unblocked_after_parent_complete() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("parent", "parent", "high", {}, "c", 1));
    scheduler.submitTask(Task("child", "child", "high", {"parent"}, "c", 1));
    Task* p = scheduler.getNextAvailableTask();
    Assert::equal(p->task_id, std::string("parent"), "Parent must be first");
    Assert::is_null(scheduler.getNextAvailableTask(), "Child must be blocked before parent completes");
    scheduler.markTaskAsCompleted("parent");
    Task* c = scheduler.getNextAvailableTask();
    Assert::not_null(c, "Child should be available after parent completes");
    Assert::equal(c->task_id, std::string("child"), "Child task should be unblocked");
}

void chain_executes_in_order() {
    Publisher pub;
    Scheduler scheduler(pub);
    scheduler.submitTask(Task("t1", "step 1", "high", {}, "c", 1));
    scheduler.submitTask(Task("t2", "step 2", "high", {"t1"}, "c", 1));
    scheduler.submitTask(Task("t3", "step 3", "high", {"t2"}, "c", 1));
    auto advance = [&](const std::string& expected_id) {
        Task* t = scheduler.getNextAvailableTask();
        Assert::not_null(t, "Expected task: " + expected_id);
        Assert::equal(t->task_id, expected_id, "Wrong task in chain");
        scheduler.markTaskAsCompleted(t->task_id);
    };
    advance("t1");
    advance("t2");
    advance("t3");
}

void empty_scheduler_returns_null() {
    Publisher pub;
    Scheduler scheduler(pub);
    Assert::is_null(scheduler.getNextAvailableTask(), "Empty scheduler must return null");
}
} // namespace TaskSchedulingSteps
