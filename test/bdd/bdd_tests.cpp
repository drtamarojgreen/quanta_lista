#include "../test_framework.h"
#include "feature/agent_lifecycle_feature.h"
#include "feature/task_scheduling_feature.h"
#include "feature/event_publishing_feature.h"
#include "feature/json_serialization_feature.h"
#include "feature/multi_agent_coordination_feature.h"
#include "feature/schedule_management_feature.h"

int main() {
    std::cout << Color::BOLD << "\n══════════════════════════════════════════\n"
              << "  QuantaLista — BDD Tests (Gherkin)\n"
              << "══════════════════════════════════════════\n"
              << Color::RESET;

    feature_agent_lifecycle();
    feature_task_scheduling();
    feature_event_publishing();
    feature_json_serialization();
    feature_multi_agent_coordination();
    feature_schedule_management();

    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
