#include "StatsService.h"

#include <QDate>
#include <QDir>

#include "DataStore.h"
#include "FocusRecordService.h"

namespace {

bool isInRange(const QDate& date, StatsRange range){
    const QDate today = QDate::currentDate();
    switch(range){
    case StatsRange::Day:
        return date == today;
    case StatsRange::Week: {
        const QDate weekStart = today.addDays(1 - today.dayOfWeek());
        return date >= weekStart && date <= today;
    }
    case StatsRange::Month:
        return date.year() == today.year() && date.month() == today.month();
    case StatsRange::Year:
        return date.year() == today.year();
    }
    return false;
}

}

StatsSummary StatsService::summarize(const QString& user, StatsRange range){
    StatsSummary summary;
    QDir dir(DataStore::userDir(user));
    const auto files = dir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Name);

    for(const auto& fi : files){
        const QDate date = QDate::fromString(fi.baseName(), "yyyy-MM-dd");
        if(!date.isValid() || !isInRange(date, range)){
            continue;
        }

        const FocusSummary daily = FocusRecordService::summarizeContent(DataStore::readAll(fi.filePath()));
        summary.pomodoroCount += daily.pomodoroCount;
        summary.stopwatchEvents += daily.stopwatchEvents;
        summary.pomodoroMinutes += daily.pomodoroMinutes;
        summary.stopwatchMinutes += daily.stopwatchMinutes;
        summary.totalFocusMinutes += daily.totalFocusMinutes;
    }

    return summary;
}
