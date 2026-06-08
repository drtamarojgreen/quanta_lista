#include "../test_framework.h"
#include "../../src/models/ModelBackend.h"
#include <fstream>
#include <cstdio>

void TestModelBackend_LoadConfig() {
    UnitTest::step("Writing temporary .quanta model backend configuration");
    {
        std::ofstream f(".quanta");
        f << "# Comment line\n";
        f << "engine.model_path = test_model.gguf\n";
        f << "model.llama_cli_path = test_cli\n";
    }

    UnitTest::step("Constructing ModelBackend from local configuration");
    ModelBackend backend;
    UnitTest::step("Verifying backend reports available with configured paths");
    Assert::is_true(backend.is_available(), "Backend should be available with config");

    UnitTest::step("Removing temporary .quanta configuration");
    std::remove(".quanta");
}

void TestModelBackend_NoConfigFallback() {
    UnitTest::step("Ensuring .quanta is absent before fallback check");
    if (std::remove(".quanta") == 0) {
        // file existed and was removed
    }

    UnitTest::step("Constructing ModelBackend without local configuration");
    ModelBackend backend;
    UnitTest::step("Verifying backend reports unavailable without configuration");
    Assert::is_true(!backend.is_available(), "Backend should not be available without config");
    UnitTest::step("Running model request through unconfigured backend");
    std::string response = backend.run_model("test");
    UnitTest::step("Verifying unconfigured backend returns the expected error message");
    Assert::equal(response, std::string("Error: Model backend not configured."), "Should return error message");
}

int main() {
    UnitTest::section("Model Backend Tests");
    UnitTest::run(TestModelBackend_LoadConfig, "TestModelBackend_LoadConfig");
    UnitTest::run(TestModelBackend_NoConfigFallback, "TestModelBackend_NoConfigFallback");
    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
