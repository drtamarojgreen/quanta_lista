#include "task_scheduling_feature.h"
#include "../../test_framework.h"
#include "../step/task_scheduling_steps.h"

void feature_task_scheduling() {
    Gherkin::feature("Task Scheduling and Priority")
        .scenario("A high-priority task is scheduled before a low-priority task")
        .given("a scheduler containing a low-priority task and a high-priority task", []() {})
        .when("the next available task is requested", []() {})
        .then("the high-priority task is returned first", []() { TaskSchedulingSteps::high_before_low(); })

        .scenario("Tasks are served in high, medium, then low priority order")
        .given("a scheduler with one task at each priority level", []() {})
        .when("tasks are dequeued one by one", []() {})
        .then("they arrive in high, medium, low order", []() { TaskSchedulingSteps::priority_order_hml(); })

        .scenario("A task with an unmet dependency is not scheduled")
        .given("a scheduler with a task whose dependency does not exist", []() {})
        .when("the next available task is requested", []() {})
        .then("no task is returned", []() { TaskSchedulingSteps::unmet_dependency_not_scheduled(); })

        .scenario("A dependent task becomes schedulable once its parent completes")
        .given("a parent task and a child task that depends on it", []() {})
        .when("the parent is completed", []() {})
        .then("the child becomes the next available task", []() { TaskSchedulingSteps::child_unblocked_after_parent_complete(); })

        .scenario("A three-task chain executes strictly in dependency order")
        .given("tasks T1, T2 (depends on T1), and T3 (depends on T2)", []() {})
        .when("tasks are dequeued and completed one at a time", []() {})
        .then("they complete in the order T1, T2, T3", []() { TaskSchedulingSteps::chain_executes_in_order(); })

        .scenario("An empty scheduler returns no tasks")
        .given("a scheduler with no submitted tasks", []() {})
        .when("the next available task is requested", []() {})
        .then("null is returned", []() { TaskSchedulingSteps::empty_scheduler_returns_null(); })
        .run();
}
