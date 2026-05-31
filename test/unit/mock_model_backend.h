#ifndef MOCK_MODEL_BACKEND_H
#define MOCK_MODEL_BACKEND_H

#include "../src/models/ModelBackend.h"
#include <string>
#include <functional>

class MockModelBackend : public ModelBackend {
public:
    std::function<std::string(const std::string&)> response_generator;
    bool available = true;

    MockModelBackend() {}

    bool is_available() const { return available; }
    
    std::string run_model(const std::string& input) {
        if (response_generator) return response_generator(input);
        return "Stubbed: " + input;
    }
};

#endif // MOCK_MODEL_BACKEND_H
