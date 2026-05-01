#include "json_serialization_steps.h"
#include "../../test_framework.h"

namespace JsonSerializationSteps {
void full_task_roundtrip() {
    Task original("t1", "Do something", "high", {"dep_a", "dep_b"}, "module_x", 60);
    Task restored = from_json(to_json(original));
    Assert::equal(restored.task_id, original.task_id, "task_id");
    Assert::equal(restored.description, original.description, "description");
    Assert::equal(restored.priority, original.priority, "priority");
    Assert::equal(restored.component, original.component, "component");
    Assert::equal(restored.max_runtime_sec, original.max_runtime_sec, "max_runtime_sec");
    Assert::size_equals(restored.dependencies, size_t(2), "dependency count");
    Assert::equal(restored.dependencies[0], std::string("dep_a"), "dependency[0]");
    Assert::equal(restored.dependencies[1], std::string("dep_b"), "dependency[1]");
}

void empty_dependencies_roundtrip() {
    Task original("t2", "Solo task", "low", {}, "c", 10);
    Task restored = from_json(to_json(original));
    Assert::is_true(restored.dependencies.empty(), "Empty dependency list should be preserved");
}

void single_dependency_roundtrip() {
    Task original("t3", "Chained task", "medium", {"sole_dep"}, "c", 5);
    Task restored = from_json(to_json(original));
    Assert::size_equals(restored.dependencies, size_t(1), "Exactly one dependency");
    Assert::equal(restored.dependencies[0], std::string("sole_dep"), "Dependency ID must match");
}
} // namespace JsonSerializationSteps
