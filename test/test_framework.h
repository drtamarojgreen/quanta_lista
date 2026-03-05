#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "QuantaLista.h"
#include "pubsub.h"
#include <any>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Color {
inline const std::string GREEN = "\033[32m";
inline const std::string RED = "\033[31m";
inline const std::string YELLOW = "\033[33m";
inline const std::string CYAN = "\033[36m";
inline const std::string BOLD = "\033[1m";
inline const std::string DIM = "\033[2m";
inline const std::string RESET = "\033[0m";
} // namespace Color

namespace TestRegistry {

struct Result {
    std::string suite;
    std::string name;
    bool passed;
    std::string failure_message;
};

inline std::vector<Result>& results() {
    static std::vector<Result> reg;
    return reg;
}
inline int& passed_count() {
    static int n = 0;
    return n;
}
inline int& failed_count() {
    static int n = 0;
    return n;
}

inline void record(const std::string& suite, const std::string& name, bool passed,
                   const std::string& msg = "") {
    results().push_back({suite, name, passed, msg});
    if (passed) {
        ++passed_count();
    } else {
        ++failed_count();
    }
}

inline void print_summary() {
    const int total = passed_count() + failed_count();
    std::cout << "\n"
              << Color::BOLD
              << "══════════════════════════════════════════\n"
              << "  Final Test Summary\n"
              << "══════════════════════════════════════════" << Color::RESET << "\n"
              << "  Total:   " << total << "\n"
              << "  " << Color::GREEN << "Passed:  " << passed_count() << Color::RESET
              << "\n";

    if (failed_count() > 0) {
        std::cout << "  " << Color::RED << "Failed:  " << failed_count() << Color::RESET
                  << "\n"
                  << "\n"
                  << Color::RED << "  Failed Tests:" << Color::RESET << "\n";
        for (const auto& r : results()) {
            if (!r.passed) {
                std::cout << "    [" << r.suite << "] " << r.name << "\n"
                          << "      → " << r.failure_message << "\n";
            }
        }
    }

    std::cout << Color::BOLD << "══════════════════════════════════════════\n"
              << Color::RESET << "\n";
}

} // namespace TestRegistry

namespace Assert {

inline void is_true(bool condition, const std::string& msg) {
    if (!condition) {
        throw std::runtime_error(msg);
    }
}

inline void is_false(bool condition, const std::string& msg) {
    if (condition) {
        throw std::runtime_error(msg);
    }
}

template <typename T>
inline void equal(const T& actual, const T& expected, const std::string& msg) {
    if (!(actual == expected)) {
        std::ostringstream oss;
        oss << msg << " (values not equal)";
        throw std::runtime_error(oss.str());
    }
}

inline void not_null(const void* ptr, const std::string& msg) {
    if (ptr == nullptr) {
        throw std::runtime_error(msg + " (expected non-null, got null)");
    }
}

inline void is_null(const void* ptr, const std::string& msg) {
    if (ptr != nullptr) {
        throw std::runtime_error(msg + " (expected null, got non-null)");
    }
}

template <typename C>
inline void size_equals(const C& container, size_t expected, const std::string& msg) {
    if (container.size() != expected) {
        std::ostringstream oss;
        oss << msg << " (expected size=" << expected << ", got size=" << container.size()
            << ")";
        throw std::runtime_error(oss.str());
    }
}

} // namespace Assert

class MockSubscriber : public ISubscriber {
public:
    std::vector<std::any> received_events;

    void onEvent(const Event& event) override {
        switch (event.type) {
        case EventType::TaskCreated:
            received_events.push_back(static_cast<const TaskCreatedEvent&>(event));
            break;
        case EventType::TaskStatusChanged:
            received_events.push_back(static_cast<const TaskStatusChangedEvent&>(event));
            break;
        case EventType::AgentStateChanged:
            received_events.push_back(static_cast<const AgentStateChangedEvent&>(event));
            break;
        }
    }

    void clear() { received_events.clear(); }

    template <typename T>
    int count_of() const {
        int n = 0;
        for (const auto& e : received_events) {
            if (e.type() == typeid(T)) {
                ++n;
            }
        }
        return n;
    }

    template <typename T>
    const T& get(size_t index) const {
        return std::any_cast<const T&>(received_events.at(index));
    }

    bool empty() const { return received_events.empty(); }
    size_t size() const { return received_events.size(); }
};

namespace UnitTest {

inline void section(const std::string& title) {
    std::cout << "\n" << Color::BOLD << Color::YELLOW << "  ── " << title << " ──"
              << Color::RESET << "\n";
}

inline void run(void (*fn)(), const std::string& name) {
    std::cout << "    " << Color::DIM << "▸ " << Color::RESET << std::left
              << std::setw(62) << name << " ";
    try {
        fn();
        std::cout << Color::GREEN << "PASS" << Color::RESET << "\n";
        TestRegistry::record("Unit", name, true);
    } catch (const std::exception& e) {
        std::cout << Color::RED << "FAIL" << Color::RESET << "\n"
                  << "        " << Color::RED << "→ " << e.what() << Color::RESET << "\n";
        TestRegistry::record("Unit", name, false, e.what());
    }
}

} // namespace UnitTest

namespace Gherkin {

struct Step {
    std::string keyword;
    std::string text;
    std::function<void()> action;
};

class FeatureBuilder;

class ScenarioBuilder {
public:
    std::string description;
    std::vector<Step> steps;
    bool passed = true;
    std::string failure_step;
    std::string failure_message;
    FeatureBuilder* parent = nullptr;

    ScenarioBuilder& given(const std::string& text, std::function<void()> fn) {
        steps.push_back({"Given", text, fn});
        return *this;
    }
    ScenarioBuilder& when(const std::string& text, std::function<void()> fn) {
        steps.push_back({"When", text, fn});
        return *this;
    }
    ScenarioBuilder& then(const std::string& text, std::function<void()> fn) {
        steps.push_back({"Then", text, fn});
        return *this;
    }
    ScenarioBuilder& and_(const std::string& text, std::function<void()> fn) {
        steps.push_back({"And", text, fn});
        return *this;
    }
    ScenarioBuilder& but(const std::string& text, std::function<void()> fn) {
        steps.push_back({"But", text, fn});
        return *this;
    }
    inline ScenarioBuilder& scenario(const std::string& description);
    inline void run();

    void execute() {
        bool skip = false;
        for (auto& step : steps) {
            if (skip) {
                std::cout << "        " << Color::DIM << std::left << std::setw(7)
                          << step.keyword << step.text << "  (skipped)" << Color::RESET
                          << "\n";
                continue;
            }
            try {
                step.action();
                std::cout << "        " << Color::DIM << std::left << std::setw(7)
                          << step.keyword << Color::RESET << step.text << "\n";
            } catch (const std::exception& e) {
                passed = false;
                failure_step = step.keyword + " " + step.text;
                failure_message = e.what();
                std::cout << "        " << Color::RED << std::left << std::setw(7)
                          << step.keyword << Color::RESET << step.text << "  "
                          << Color::RED << "✘ FAILED" << Color::RESET << "\n"
                          << "          " << Color::RED << "→ " << e.what()
                          << Color::RESET << "\n";
                skip = true;
            }
        }
    }
};

class FeatureBuilder {
public:
    std::string name;
    std::vector<ScenarioBuilder*> scenarios;

    ~FeatureBuilder() {
        for (auto* s : scenarios) {
            delete s;
        }
    }

    ScenarioBuilder& scenario(const std::string& description) {
        auto* s = new ScenarioBuilder();
        s->description = description;
        s->parent = this;
        scenarios.push_back(s);
        return *s;
    }

    void run() {
        std::cout << "\n" << Color::BOLD << Color::CYAN << "Feature: " << name
                  << Color::RESET << "\n";

        for (auto* s : scenarios) {
            std::cout << "\n    " << Color::BOLD << "Scenario: " << s->description
                      << Color::RESET << "\n";
            s->execute();

            const std::string full_name = name + " > " + s->description;
            if (s->passed) {
                std::cout << "      " << Color::GREEN << "✔ Passed" << Color::RESET << "\n";
                TestRegistry::record("BDD", full_name, true);
            } else {
                std::cout << "      " << Color::RED << "✘ Failed" << Color::RESET << "\n";
                TestRegistry::record("BDD", full_name, false,
                                     s->failure_step + " → " + s->failure_message);
            }
        }
    }
};

inline ScenarioBuilder& ScenarioBuilder::scenario(const std::string& description) {
    return parent->scenario(description);
}

inline void ScenarioBuilder::run() { parent->run(); }

inline FeatureBuilder& feature(const std::string& name) {
    auto* f = new FeatureBuilder();
    f->name = name;
    return *f;
}

} // namespace Gherkin

#endif
