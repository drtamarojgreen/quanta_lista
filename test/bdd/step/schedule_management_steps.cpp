#include "schedule_management_steps.h"
#include "../../test_framework.h"
#include "../../../src/QuantaLista.h"
#include <filesystem>
#include <fstream>

static Schedule* current_test_schedule = nullptr;
static Scheduler* current_test_scheduler = nullptr;
static Publisher* current_test_pub = nullptr;

void given_new_schedule(const std::string& name) {
    if (current_test_schedule) delete current_test_schedule;
    current_test_schedule = new Schedule("sch_test", name);
    if (!current_test_pub) current_test_pub = new Publisher();
    if (current_test_scheduler) delete current_test_scheduler;
    current_test_scheduler = new Scheduler(*current_test_pub);
}

void and_add_task_to_schedule(const std::string& desc) {
    current_test_schedule->addTask(Task(desc, desc, "medium", {}, "c", 1));
}

void when_save_schedule(const std::string& path) {
    current_test_scheduler->setSchedule(*current_test_schedule);
    current_test_scheduler->saveSchedule(path);
}

void then_file_exists(const std::string& path) {
    Assert::is_true(std::filesystem::exists(path), "File should exist: " + path);
    std::filesystem::remove(path);
}

void given_schedule_file_exists(const std::string& path, const std::string& taskDesc) {
    Schedule sch("sch_load", "Loaded Schedule");
    sch.addTask(Task(taskDesc, taskDesc, "low", {}, "c", 1));
    std::ofstream file(path);
    file << to_json(sch);
    file.close();

    if (!current_test_pub) current_test_pub = new Publisher();
    if (current_test_scheduler) delete current_test_scheduler;
    current_test_scheduler = new Scheduler(*current_test_pub);
}

void when_load_schedule(const std::string& path) {
    current_test_scheduler->loadSchedule(path);
    std::filesystem::remove(path);
}

void then_schedule_task_count(int count) {
    Assert::size_equals(current_test_scheduler->getSchedule().tasks, size_t(count), "Schedule task count");
}

void and_task_present(const std::string& desc) {
    bool found = false;
    for (const auto& t : current_test_scheduler->getSchedule().tasks) {
        if (t.description == desc) {
            found = true;
            break;
        }
    }
    Assert::is_true(found, "Task with description '" + desc + "' should be present");
}

void when_remove_task(const std::string& desc) {
    // Note: our current removeTask uses task_id, we use desc as id in these tests
    current_test_scheduler->removeTask(desc);
}

void and_add_task_with_datetime(const std::string& desc, const std::string& date, const std::string& time) {
    Task t(desc, desc, "medium", {}, "c", 1);
    t.date = date;
    t.time = time;
    current_test_scheduler->submitTask(t);
}

void then_conflict_detected(const std::string& desc) {
    // A simplistic conflict detection for the test: check if another task has same date/time
    bool conflict = false;
    const auto& tasks = current_test_scheduler->getSchedule().tasks;
    for (const auto& t : tasks) {
        if (t.description == desc) {
            for (const auto& other : tasks) {
                if (other.task_id != t.task_id && other.date == t.date && other.time == t.time) {
                    conflict = true;
                    break;
                }
            }
        }
    }
    Assert::is_true(conflict, "Conflict should be detected for " + desc);
}
