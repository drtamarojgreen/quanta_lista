#include "../test_framework.h"
#include "fact_utils.h"
#include "../../src/QuantaLista.h"
#include "../../src/SchedulerUI.h"

using namespace Chai::Cdd::Util;

// @Card: validate_scheduler_logic
void validate_scheduler_logic(const std::map<std::string, std::string>& facts) {
    Publisher pub;
    Scheduler scheduler(pub);
    Greenhouse::UI::SchedulerUI ui;

    if (facts.count("InitialTaskCount")) {
        size_t actualCount = scheduler.getSchedule().tasks.size();
        std::cout << "result_InitialTaskCount = " << actualCount << std::endl;
    }

    if (facts.count("TaskPriority")) {
        Task t1("t1", "Task 1", facts.at("TaskPriority"), {}, "c", 1);
        scheduler.submitTask(t1);
        Task* next = scheduler.getNextAvailableTask();

        if (next) {
            std::cout << "result_TaskPriority = " << next->priority << std::endl;
        } else {
            std::cout << "result_TaskPriority = NONE" << std::endl;
        }
    }

    if (facts.count("LeapYearDate")) {
        std::cout << "result_LeapYearDay = " << ui.renderPatientCalendar(2024, 2) << std::endl;
    }

    if (facts.count("InvalidDate")) {
        // This might crash or return a default value depending on implementation
        // Verifying robustness
        std::cout << "result_InvalidDateDay = " << ui.renderPatientCalendar(2025, 13) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <facts_file>" << std::endl;
        return 1;
    }
    auto facts = FactReader::readFacts(argv[1]);
    validate_scheduler_logic(facts);
    return 0;
}
