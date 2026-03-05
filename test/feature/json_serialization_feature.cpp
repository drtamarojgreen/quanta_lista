#include "json_serialization_feature.h"
#include "../test_framework.h"
#include "../step/json_serialization_steps.h"

void feature_json_serialization() {
    Gherkin::feature("Task JSON Serialization")
        .scenario("A fully-populated task survives a JSON round-trip without data loss")
        .given("a Task with ID, description, priority, component, runtime, and two dependencies", []() {})
        .when("the task is serialised to JSON and deserialised back", []() {})
        .then("all fields on the restored task match the original", []() { JsonSerializationSteps::full_task_roundtrip(); })

        .scenario("A task with no dependencies round-trips without inventing dependencies")
        .given("a Task with an empty dependencies list", []() {})
        .when("the task is serialised and deserialised", []() {})
        .then("the restored task's dependency list is empty", []() { JsonSerializationSteps::empty_dependencies_roundtrip(); })

        .scenario("A task with a single dependency preserves that dependency exactly")
        .given("a Task with exactly one dependency ID", []() {})
        .when("the task is serialised and deserialised", []() {})
        .then("the restored task has exactly one dependency with the correct ID", []() { JsonSerializationSteps::single_dependency_roundtrip(); })
        .run();
}
