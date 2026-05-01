#ifndef SCHEDULER_UI_H
#define SCHEDULER_UI_H

#include <string>
#include <vector>
#include <map>
#include "QuantaLista.h"

namespace Greenhouse {
namespace UI {

class SchedulerUI {
public:
    // View Management
    std::string renderViewSelector(const std::string& currentView);

    // Patient View
    std::string renderPatientForm();
    std::string renderPatientCalendar(int year, int month);
    std::string renderInstructionsPanel();
    std::string renderPatientAppointmentList(const std::vector<Task>& appointments);

    // Dashboard View
    std::string renderDashboardWeeklySchedule(const Schedule& schedule, const std::string& startDate);
    std::string renderDashboardCalendar(int year, int month);
    std::string renderConflictList(const std::vector<std::string>& conflicts);
    std::string renderNewAppointmentBox();

    // Admin View
    std::string renderAdminSettingsForm();
    std::string renderAdminAppointmentEditor(const Task& task);

    // Common Components
    std::string renderConflictModal(const std::string& taskId, const std::vector<std::string>& overlappingTasks);
    std::string renderLoadingSpinner(const std::string& message);
    std::string renderNotification(const std::string& message, const std::string& type);

private:
    std::string getMonthName(int month);
    std::string formatTime(int hour);
    int calculateDayOfWeek(const std::string& dateStr); // YYYY-MM-DD
};

} // namespace UI
} // namespace Greenhouse

#endif // SCHEDULER_UI_H
