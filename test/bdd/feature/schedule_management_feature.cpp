#include "schedule_management_feature.h"
#include "../../test_framework.h"
#include "../step/schedule_management_steps.h"

void feature_schedule_management() {
    Feature("Schedule Management", "As a user, I want to manage a collection of tasks as a single schedule.");

    Scenario("Creating and saving a new schedule")
        .Given("I have a new schedule named 'Morning Routine'")
        .And("I add a task 'Wake up' to the schedule")
        .When("I save the schedule to 'morning.json'")
        .Then("The file 'morning.json' should exist");

    Scenario("Loading a schedule from a file")
        .Given("A schedule file 'evening.json' exists with task 'Sleep'")
        .When("I load the schedule from 'evening.json'")
        .Then("The current schedule should have 1 task")
        .And("The task 'Sleep' should be present");

    Scenario("Removing a task from the schedule")
        .Given("I have a new schedule named 'Gym'")
        .And("I add a task 'Squats' to the schedule")
        .When("I remove the task 'Squats'")
        .Then("The current schedule should have 0 tasks");

    Scenario("Removing a non-existent task")
        .Given("I have a new schedule named 'Empty'")
        .When("I remove the task 'Ghost'")
        .Then("The current schedule should have 0 tasks");

    Scenario("Detecting scheduling conflicts")
        .Given("I have a new schedule named 'Busy Day'")
        .And("I add a task 'Meeting A' on '2025-05-12' at '10:00'")
        .And("I add a task 'Meeting B' on '2025-05-12' at '10:00'")
        .Then("A conflict should be detected for 'Meeting B'");
}
