#include "schedule_management_feature.h"
#include "../../test_framework.h"
#include "../step/schedule_management_steps.h"

void feature_schedule_management() {
    Gherkin::feature("Schedule Management")
        .scenario("Creating and saving a new schedule")
        .given("I have a new schedule named 'Morning Routine'", []() { given_new_schedule("Morning Routine"); })
        .and_("I add a task 'Wake up' to the schedule", []() { and_add_task_to_schedule("Wake up"); })
        .when("I save the schedule to 'morning.json'", []() { when_save_schedule("morning.json"); })
        .then("The file 'morning.json' should exist", []() { then_file_exists("morning.json"); })

        .scenario("Loading a schedule from a file")
        .given("A schedule file 'evening.json' exists with task 'Sleep'", []() { given_schedule_file_exists("evening.json", "Sleep"); })
        .when("I load the schedule from 'evening.json'", []() { when_load_schedule("evening.json"); })
        .then("The current schedule should have 1 task", []() { then_schedule_task_count(1); })
        .and_("The task 'Sleep' should be present", []() { and_task_present("Sleep"); })

        .scenario("Removing a task from the schedule")
        .given("I have a new schedule named 'Gym'", []() { given_new_schedule("Gym"); })
        .and_("I add a task 'Squats' to the schedule", []() { and_add_task_to_schedule("Squats"); })
        .when("I remove the task 'Squats'", []() { when_remove_task("Squats"); })
        .then("The current schedule should have 0 tasks", []() { then_schedule_task_count(0); })

        .scenario("Removing a non-existent task")
        .given("I have a new schedule named 'Empty'", []() { given_new_schedule("Empty"); })
        .when("I remove the task 'Ghost'", []() { when_remove_task("Ghost"); })
        .then("The current schedule should have 0 tasks", []() { then_schedule_task_count(0); })

        .scenario("Detecting scheduling conflicts")
        .given("I have a new schedule named 'Busy Day'", []() { given_new_schedule("Busy Day"); })
        .and_("I add a task 'Meeting A' on '2025-05-12' at '10:00'", []() { and_add_task_with_datetime("Meeting A", "2025-05-12", "10:00"); })
        .and_("I add a task 'Meeting B' on '2025-05-12' at '10:00'", []() { and_add_task_with_datetime("Meeting B", "2025-05-12", "10:00"); })
        .then("A conflict should be detected for 'Meeting B'", []() { then_conflict_detected("Meeting B"); })
        .run();
}
