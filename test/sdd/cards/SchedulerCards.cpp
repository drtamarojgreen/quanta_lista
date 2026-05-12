#include "../../test_framework.h"
#include "fact_utils.h"
#include "../../src/QuantaLista.h"
#include "../../src/SchedulerUI.h"

using namespace Sorrel::Sdd::Util;

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

    if (facts.count("HasLabels")) {
        Task t("label_task", "Task with labels", "low", {}, "c", 1);
        t.labels = {"urgent", "work"};
        std::string json = to_json(t);
        Task restored = from_json(json);
        std::cout << "result_LabelCount = " << restored.labels.size() << std::endl;
        for (const auto& l : restored.labels) {
            std::cout << "result_LabelValue = " << l << std::endl;
        }
    }

    if (facts.count("SortInput")) {
        // [t3:t2, t2:t1, t1:]
        std::vector<Task> tasks;
        tasks.emplace_back("t3", "d", "m", std::vector<std::string>{"t2"}, "c", 1);
        tasks.emplace_back("t1", "d", "m", std::vector<std::string>{}, "c", 1);
        tasks.emplace_back("t2", "d", "m", std::vector<std::string>{"t1"}, "c", 1);

        auto sorted = scheduler.getTopologicallySortedTasks(tasks);
        std::cout << "result_SortOrder = ";
        for (size_t i = 0; i < sorted.size(); ++i) {
            std::cout << sorted[i].task_id << (i == sorted.size() - 1 ? "" : ",");
        }
        std::cout << std::endl;
    }

    if (facts.count("InvalidTaskID")) {
        Task t("", "desc", "low", {}, "c", 1);
        std::cout << "result_ValidateEmptyID = " << (scheduler.validateTask(t) ? "VALID" : "INVALID") << std::endl;
    }

    if (facts.count("ValidTaskID") && facts.count("ValidTaskDesc")) {
        Task t(facts.at("ValidTaskID"), facts.at("ValidTaskDesc"), "low", {}, "c", 1);
        std::cout << "result_ValidateValidTask = " << (scheduler.validateTask(t) ? "VALID" : "INVALID") << std::endl;
    }

    if (facts.count("DuplicateID")) {
        std::string id = facts.at("DuplicateID");
        Task t1(id, "first", "low", {}, "c", 1);
        scheduler.submitTask(t1);

        std::string json = "{\"name\": \"dup\", \"schedule_id\": \"s1\", \"tasks\": [{\"task_id\": \"" + id + "\", \"description\": \"second\"}]}";
        scheduler.importFromJSON(json);

        std::cout << "result_DuplicateDetectedCount = " << scheduler.getSchedule().tasks.size() << std::endl;
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
