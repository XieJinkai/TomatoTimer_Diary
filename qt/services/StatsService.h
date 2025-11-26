#pragma once
#include <QString>

struct StatsSummary {
    int pomodoroCountWeek{0};
    int pomodoroCountMonth{0};
    int totalFocusMinutesWeek{0};
    int totalFocusMinutesMonth{0};
    int stopwatchEventsWeek{0};
    int stopwatchEventsMonth{0};
};

class StatsService {
public:
    static StatsSummary summarize(const QString& user);
};