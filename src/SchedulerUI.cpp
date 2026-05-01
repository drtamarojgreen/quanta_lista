#include "SchedulerUI.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Greenhouse {
namespace UI {

std::string SchedulerUI::renderViewSelector(const std::string& currentView) {
    std::stringstream ss;
    ss << "╔══════════════════════════════════════════╗\n";
    ss << "║             SELECT VIEW                  ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║  " << (currentView == "patient" ? "[●]" : "[ ]") << " Patient View                      ║\n";
    ss << "║  " << (currentView == "dashboard" ? "[●]" : "[ ]") << " Dashboard View                    ║\n";
    ss << "║  " << (currentView == "admin" ? "[●]" : "[ ]") << " Admin View                        ║\n";
    ss << "╚══════════════════════════════════════════╝\n";
    return ss.str();
}

std::string SchedulerUI::renderPatientForm() {
    std::stringstream ss;
    ss << "╔══════════════════════════════════════════╗\n";
    ss << "║        REQUEST AN APPOINTMENT            ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║ Title: ________________________________  ║\n";
    ss << "║ Date:  ____-____-____ (YYYY-MM-DD)      ║\n";
    ss << "║ Time:  ____:____ (HH:MM)                ║\n";
    ss << "║ Platform: _____________________________  ║\n";
    ss << "║ Service: [ Choose a service...        ▼] ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║          [ REQUEST APPOINTMENT ]         ║\n";
    ss << "╚══════════════════════════════════════════╝\n";
    return ss.str();
}

std::string SchedulerUI::renderPatientCalendar(int year, int month) {
    std::stringstream ss;
    ss << "       " << getMonthName(month) << " " << year << "\n";
    ss << "  S   M   T   W   T   F   S\n";
    ss << "─────────────────────────────\n";

    // Zeller's congruence to find the first day of the month
    int q = 1; // 1st day
    int m = month;
    int y = year;
    if (m < 3) {
        m += 12;
        y--;
    }
    int k = y % 100;
    int j = y / 100;
    int h = (q + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;

    // Convert h (Saturday=0, Sunday=1...) to (Sunday=0, Monday=1...)
    int firstDay = (h + 6) % 7;

    // Number of days in month
    int daysInMonth = 31;
    if (month == 4 || month == 6 || month == 9 || month == 11) daysInMonth = 30;
    else if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        daysInMonth = isLeap ? 29 : 28;
    }

    // Print leading spaces
    for (int i = 0; i < firstDay; ++i) ss << "    ";

    for (int day = 1; day <= daysInMonth; ++day) {
        ss << std::setw(3) << day << " ";
        if ((day + firstDay) % 7 == 0) ss << "\n";
    }

    if ((daysInMonth + firstDay) % 7 != 0) ss << "\n";
    ss << "─────────────────────────────\n";
    return ss.str();
}

std::string SchedulerUI::renderInstructionsPanel() {
    std::stringstream ss;
    ss << "╔══════════════════════════════════════════╗\n";
    ss << "║    HOW TO REQUEST AN APPOINTMENT         ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║ 1. Fill Out the Form                     ║\n";
    ss << "║    Complete all required fields.         ║\n";
    ss << "║ 2. Review Your Request                   ║\n";
    ss << "║    Ensure accuracy before submitting.    ║\n";
    ss << "║ 3. Wait for Confirmation                 ║\n";
    ss << "║    We respond within 24 hours.           ║\n";
    ss << "║ 4. Need Help?                            ║\n";
    ss << "║    Contact our office directly.          ║\n";
    ss << "╚══════════════════════════════════════════╝\n";
    return ss.str();
}

std::string SchedulerUI::renderPatientAppointmentList(const std::vector<Task>& appointments) {
    std::stringstream ss;
    ss << "MY APPOINTMENTS\n";
    ss << "------------------------------------------\n";
    if (appointments.empty()) {
        ss << "No appointments scheduled.\n";
    } else {
        for (const auto& appt : appointments) {
            ss << "- " << appt.task_id << ": " << appt.description << " [" << appt.priority << "]\n";
        }
    }
    return ss.str();
}

std::string SchedulerUI::renderDashboardWeeklySchedule(const Schedule& schedule, const std::string& startDate) {
    std::stringstream ss;
    ss << "WEEKLY SCHEDULE (Starting " << startDate << ")\n";
    ss << "──────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐\n";
    ss << " TIME │ SUN │ MON │ TUE │ WED │ THU │ FRI │ SAT │\n";
    ss << "──────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤\n";

    std::map<int, std::map<int, std::string>> grid; // hour -> day -> label
    for (const auto& task : schedule.tasks) {
        if (!task.date.empty() && !task.time.empty()) {
            int day = calculateDayOfWeek(task.date);
            int hour = std::stoi(task.time.substr(0, 2));
            if (hour >= 8 && hour <= 17) {
                grid[hour][day] = "[X]";
            }
        }
    }

    for (int hour = 8; hour <= 17; ++hour) {
        ss << " " << formatTime(hour) << " │";
        for (int day = 0; day < 7; ++day) {
            std::string mark = grid[hour].count(day) ? grid[hour][day] : "     ";
            ss << mark << "│";
        }
        ss << "\n";
        if (hour < 17) ss << "──────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤\n";
    }
    ss << "──────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘\n";
    return ss.str();
}

std::string SchedulerUI::renderDashboardCalendar(int year, int month) {
    return renderPatientCalendar(year, month);
}

std::string SchedulerUI::renderConflictList(const std::vector<std::string>& conflicts) {
    std::stringstream ss;
    ss << "CONFLICTS TO RESOLVE\n";
    if (conflicts.empty()) {
        ss << "No conflicts found.\n";
    } else {
        for (const auto& conflict : conflicts) {
            ss << "[!] " << conflict << "\n";
        }
    }
    return ss.str();
}

std::string SchedulerUI::renderNewAppointmentBox() {
    return "[ NEW APPOINTMENT (DRAG ME) ]";
}

std::string SchedulerUI::renderAdminSettingsForm() {
    std::stringstream ss;
    ss << "╔══════════════════════════════════════════╗\n";
    ss << "║            ADMIN SETTINGS                ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║ Max Concurrent Appts: [ 3 ]              ║\n";
    ss << "║ Auto-Confirmation:    [ On ]             ║\n";
    ss << "║ Notification Email:   [ admin@gh.org ]   ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║             [ SAVE SETTINGS ]            ║\n";
    ss << "╚══════════════════════════════════════════╝\n";
    return ss.str();
}

std::string SchedulerUI::renderAdminAppointmentEditor(const Task& task) {
    std::stringstream ss;
    ss << "EDIT APPOINTMENT: " << task.task_id << "\n";
    ss << "Title: " << task.description << "\n";
    ss << "Priority: " << task.priority << "\n";
    ss << "Component: " << task.component << "\n";
    ss << "Max Runtime: " << task.max_runtime_sec << "s\n";
    ss << "[ SAVE CHANGES ] [ DELETE APPOINTMENT ]\n";
    return ss.str();
}

std::string SchedulerUI::renderConflictModal(const std::string& taskId, const std::vector<std::string>& overlappingTasks) {
    std::stringstream ss;
    ss << "╔══════════════════════════════════════════╗\n";
    ss << "║      SCHEDULING CONFLICT DETECTED        ║\n";
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║ Task " << taskId << " overlaps with:           ║\n";
    for (const auto& t : overlappingTasks) {
        ss << "║ - " << std::left << std::setw(34) << t << " ║\n";
    }
    ss << "╠══════════════════════════════════════════╣\n";
    ss << "║  [ CANCEL ] [ CHOOSE DIFFERENT TIME ]    ║\n";
    ss << "╚══════════════════════════════════════════╝\n";
    return ss.str();
}

std::string SchedulerUI::renderLoadingSpinner(const std::string& message) {
    return "/ Processing " + message + "...";
}

std::string SchedulerUI::renderNotification(const std::string& message, const std::string& type) {
    std::string prefix = (type == "success") ? "[OK]" : (type == "error" ? "[ERR]" : "[INFO]");
    return prefix + " " + message;
}

std::string SchedulerUI::getMonthName(int month) {
    const char* months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    if (month >= 1 && month <= 12) return months[month - 1];
    return "Unknown";
}

std::string SchedulerUI::formatTime(int hour) {
    std::stringstream ss;
    int h = (hour % 12 == 0) ? 12 : (hour % 12);
    ss << std::setw(2) << h << (hour < 12 ? " AM" : " PM");
    return ss.str();
}

int SchedulerUI::calculateDayOfWeek(const std::string& dateStr) {
    if (dateStr.length() < 10) return 0;
    int year = std::stoi(dateStr.substr(0, 4));
    int month = std::stoi(dateStr.substr(5, 2));
    int day = std::stoi(dateStr.substr(8, 2));

    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    int h = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (h + 6) % 7; // Sunday=0, Monday=1...
}

} // namespace UI
} // namespace Greenhouse
