#ifndef SCHEDULE_MANAGEMENT_STEPS_H
#define SCHEDULE_MANAGEMENT_STEPS_H
#include <string>

// Steps for Schedule Management Feature
void given_new_schedule(const std::string& name);
void and_add_task_to_schedule(const std::string& desc);
void when_save_schedule(const std::string& path);
void then_file_exists(const std::string& path);
void given_schedule_file_exists(const std::string& path, const std::string& taskDesc);
void when_load_schedule(const std::string& path);
void then_schedule_task_count(int count);
void and_task_present(const std::string& desc);
void when_remove_task(const std::string& desc);
void and_add_task_with_datetime(const std::string& desc, const std::string& date, const std::string& time);
void then_conflict_detected(const std::string& desc);

#endif
