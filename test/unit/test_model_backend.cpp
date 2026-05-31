#include "../test_framework.h"
#include "../../src/models/ModelBackend.h"
#include <fstream>
#include <cstdio>

void TestModelBackend_LoadConfig() {
    // Create a temporary .quanta file
    {
        std::ofstream f(".quanta");
        f << "# Comment line\n";
        f << "engine.model_path = test_model.gguf\n";
        f << "model.llama_cli_path = test_cli\n";
    }

    ModelBackend backend;
    Assert::is_true(backend.is_available(), "Backend should be available with config");

    std::remove(".quanta");
}

void TestModelBackend_NoConfigFallback() {
    if (std::remove(".quanta") == 0) {
        // file existed and was removed
    }

    ModelBackend backend;
    Assert::is_true(!backend.is_available(), "Backend should not be available without config");
    std::string response = backend.run_model("test");
    Assert::equal(response, std::string("Error: Model backend not configured."), "Should return error message");
}

int main() {
    UnitTest::section("Model Backend Tests");
    UnitTest::run(TestModelBackend_LoadConfig, "TestModelBackend_LoadConfig");
    UnitTest::run(TestModelBackend_NoConfigFallback, "TestModelBackend_NoConfigFallback");
    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
