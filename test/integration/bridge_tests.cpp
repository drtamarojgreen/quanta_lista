#include "../test_framework.h"
#include "../../src/models/ModelBackend.h"
#include "../../test/unit/mock_model_backend.h"
#include <fstream>
#include <cstdio>

// Mocking an Ethos-like validator for integration testing
struct MockEthosValidator {
    bool is_ethical(const std::string& input) {
        return input.find("UNETHICAL") == std::string::npos;
    }
};

void TestBridge_ListaToModelIntegration() {
    UnitTest::step("Creating mock model backend");
    MockModelBackend mock_backend;
    UnitTest::step("Installing deterministic response generator");
    mock_backend.response_generator = [](const std::string& input) {
        return "Processed: " + input;
    };

    UnitTest::step("Submitting Lista task description to model backend abstraction");
    std::string task_desc = "Implement new feature";
    std::string output = mock_backend.run_model(task_desc);
    
    UnitTest::step("Verifying backend output is returned to Lista caller");
    Assert::equal(output, std::string("Processed: Implement new feature"), "Backend integration failure");
}

void TestBridge_EthosIntegration() {
    UnitTest::step("Creating mock Ethos validator");
    MockEthosValidator ethos;
    
    UnitTest::step("Preparing safe and unsafe input examples");
    std::string safe_input = "Implement clean code";
    std::string unsafe_input = "UNETHICAL malicious task";
    
    UnitTest::step("Verifying safe input is accepted");
    Assert::is_true(ethos.is_ethical(safe_input), "Should allow safe input");
    UnitTest::step("Verifying unsafe input is rejected");
    Assert::is_true(!ethos.is_ethical(unsafe_input), "Should flag unethical input");
}

int main() {
    UnitTest::section("Integration Bridge Tests");
    UnitTest::run(TestBridge_ListaToModelIntegration, "TestBridge_ListaToModelIntegration");
    UnitTest::run(TestBridge_EthosIntegration, "TestBridge_EthosIntegration");
    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
