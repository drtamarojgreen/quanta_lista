#ifndef SCHEDULER_UI_H
#define SCHEDULER_UI_H

#include <string>
#include <vector>
#include <map>
#include "../models/models.h"

namespace Greenhouse {
namespace UI {

class SchedulerUI {
public:
    std::string renderViewSelector(const std::string& currentView);
    std::string renderPatientForm();
    std::string renderPatientCalendar(int year, int month);
    std::string renderInstructionsPanel();
    std::string renderPatientAppointmentList(const std::vector<Task>& appointments);
    std::string renderDashboardWeeklySchedule(const Schedule& schedule, const std::string& startDate);
    std::string renderDashboardCalendar(int year, int month);
    std::string renderConflictList(const std::vector<std::string>& conflicts);
    std::string renderNewAppointmentBox();
    std::string renderAdminSettingsForm();
    std::string renderAdminAppointmentEditor(const Task& task);
    std::string renderConflictModal(const std::string& taskId, const std::vector<std::string>& overlappingTasks);
    std::string renderLoadingSpinner(const std::string& message);
    std::string renderNotification(const std::string& message, const std::string& type);
    std::string renderDashboard(const Schedule& schedule, const std::vector<Agent>& agents);

private:
    std::string getMonthName(int month);
    std::string formatTime(int hour);
    int calculateDayOfWeek(const std::string& dateStr);
};

} // namespace UI
} // namespace Greenhouse

#endif // SCHEDULER_UI_H
