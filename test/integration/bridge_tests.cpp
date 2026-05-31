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
    MockModelBackend mock_backend;
    mock_backend.response_generator = [](const std::string& input) {
        return "Processed: " + input;
    };

    std::string task_desc = "Implement new feature";
    std::string output = mock_backend.run_model(task_desc);
    
    Assert::equal(output, std::string("Processed: Implement new feature"), "Backend integration failure");
}

void TestBridge_EthosIntegration() {
    MockEthosValidator ethos;
    
    std::string safe_input = "Implement clean code";
    std::string unsafe_input = "UNETHICAL malicious task";
    
    Assert::is_true(ethos.is_ethical(safe_input), "Should allow safe input");
    Assert::is_true(!ethos.is_ethical(unsafe_input), "Should flag unethical input");
}

int main() {
    UnitTest::section("Integration Bridge Tests");
    UnitTest::run(TestBridge_ListaToModelIntegration, "TestBridge_ListaToModelIntegration");
    UnitTest::run(TestBridge_EthosIntegration, "TestBridge_EthosIntegration");
    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
