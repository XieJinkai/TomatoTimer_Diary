#include "StatsService.h"
#include "DataStore.h"
#include <QDir>
#include <QDate>
#include <QFile>
#include <QTextStream>

StatsSummary StatsService::summarize(const QString& user){
    StatsSummary s;
    QDir dir(DataStore::userDir(user));
    auto files = dir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Name);
    QDate today = QDate::currentDate();
    QDate weekStart = today.addDays(-(today.dayOfWeek()%7));
    for(const auto& fi : files){
        QDate d = QDate::fromString(fi.baseName(), "yyyy-MM-dd");
        bool inWeek = d >= weekStart && d <= today;
        bool inMonth = d.month() == today.month() && d.year() == today.year();
        QFile f(fi.filePath()); if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) continue;
        QTextStream in(&f);
        while(!in.atEnd()){
            const QString line = in.readLine();
            if(line.startsWith("[Pomodoro]")){
                int mins = line.split(" ").last().replace("min"," ").trimmed().toInt();
                if(inWeek){ s.pomodoroCountWeek++; s.totalFocusMinutesWeek += mins; }
                if(inMonth){ s.pomodoroCountMonth++; s.totalFocusMinutesMonth += mins; }
            } else if(line.startsWith("[Stopwatch]")){
                if(inWeek) s.stopwatchEventsWeek++;
                if(inMonth) s.stopwatchEventsMonth++;
            }
        }
    }
    return s;
}