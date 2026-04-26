#pragma once

#include <QString>

enum class StatsRange {
    Day,
    Week,
    Month,
    Year
};

struct StatsSummary {
    int pomodoroCount{0};
    int stopwatchEvents{0};
    int totalFocusMinutes{0};
    int pomodoroMinutes{0};
    int stopwatchMinutes{0};
};

class StatsService {
public:
    static StatsSummary summarize(const QString& user, StatsRange range);
};
