#include "StatsService.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "DataStore.h"

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

        QFile file(fi.filePath());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            continue;
        }

        QTextStream in(&file);
        int pomodoroMinutes = 0;
        int stopwatchMinutes = 0;
        int genericMinutes = 0;
        int pomodoroCount = 0;
        int stopwatchCount = 0;

        while(!in.atEnd()){
            const QString line = in.readLine();
            if(line.startsWith("[Pomodoro]")){
                int mins = line.split(" ").last().replace("min", " ").trimmed().toInt();
                pomodoroMinutes += mins;
                pomodoroCount++;
            } else if(line.startsWith("[Stopwatch]")){
                const QStringList parts = line.split(' ');
                bool ok = false;
                int seconds = 0;
                if(parts.size() >= 2){
                    seconds = parts.at(1).toInt(&ok);
                }
                if(ok){
                    stopwatchMinutes += (seconds + 59) / 60;
                }
                stopwatchCount++;
            } else if(line.contains("专注时长：")){
                const int index = line.indexOf("专注时长：");
                if(index >= 0){
                    QString sub = line.mid(index + QString("专注时长：").size());
                    const int end = sub.indexOf("；");
                    const QString num = end >= 0 ? sub.left(end) : sub;
                    bool ok = false;
                    const int minutes = num.trimmed().toInt(&ok);
                    if(ok){
                        genericMinutes += minutes;
                    }
                }
            }
        }

        summary.pomodoroCount += pomodoroCount;
        summary.stopwatchEvents += stopwatchCount;
        summary.pomodoroMinutes += pomodoroMinutes;
        summary.stopwatchMinutes += stopwatchMinutes;
        summary.totalFocusMinutes += (pomodoroMinutes + stopwatchMinutes) > 0
            ? (pomodoroMinutes + stopwatchMinutes)
            : genericMinutes;
    }

    return summary;
}
