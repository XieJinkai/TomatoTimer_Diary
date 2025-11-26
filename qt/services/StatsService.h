#pragma once
#include <QString>

struct StatsSummary {
    int pomodoroCountWeek{0};
    int pomodoroCountMonth{0};
    int totalFocusMinutesWeek{0};
    int totalFocusMinutesMonth{0};
    int stopwatchEventsWeek{0};
    int stopwatchEventsMonth{0};
    int pomodoroMinutesWeek{0};
    int pomodoroMinutesMonth{0};
    int stopwatchMinutesWeek{0};
    int stopwatchMinutesMonth{0};
};

class StatsService {
public:
    // 汇总用户的周/月统计数据
    static StatsSummary summarize(const QString& user);
};
