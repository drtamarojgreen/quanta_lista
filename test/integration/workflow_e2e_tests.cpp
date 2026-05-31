#include "../test_framework.h"
#include "../../src/core/core.h"
#include "../../src/models/ModelBackend.h"
#include "../../src/utils/json_utils.h"
#include <fstream>
#include <cstdio>
#include <filesystem>

// E2E Test: Full Workflow Simulation with Real Components
void TestE2E_WorkflowIngestionToDocumentation() {
    // 1. Setup real environment
    std::filesystem::create_directories("./queue/pending");
    std::filesystem::create_directories("./queue/in_progress");
    std::filesystem::create_directories("./queue/completed");
    std::filesystem::create_directories("./queue/failed");

    // 2. Define artifacts
    Task t1("e2e-001", "Analyze project architecture", "high", {}, "analysis", 10);
    std::string task_json = to_json(t1);
    std::ofstream f("./queue/pending/e2e-001.json");
    f << task_json;
    f.close();

    // 3. Setup Project/Coordinator (using real components)
    Project p("p1", "Test Project");
    Coordinator coordinator(p, "./queue");
    
    // 4. Run Process
    // The Coordinator will now trigger the real ModelBackend if .quanta is present.
    coordinator.processPendingTasks();
    
    // 5. Verify results
    Assert::is_true(std::filesystem::exists("./queue/completed/e2e-001.json"), "Task should be in completed directory");
    
    // Check if real output file exists and has content
    std::ifstream result_file("./queue/completed/e2e-001.json");
    std::string result_content((std::istreambuf_iterator<char>(result_file)), std::istreambuf_iterator<char>());
    result_file.close();
    
    Assert::is_true(result_content.find("status") != std::string::npos, "Result should contain completion status");
    
    // 6. Cleanup
    std::filesystem::remove_all("./queue");
}

int main() {
    UnitTest::section("E2E Workflow Validation Tests (Real Components)");
    UnitTest::run(TestE2E_WorkflowIngestionToDocumentation, "TestE2E_WorkflowIngestionToDocumentation");
    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
