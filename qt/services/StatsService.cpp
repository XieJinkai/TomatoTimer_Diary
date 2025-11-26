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

        int pomMin = 0, swMin = 0, genericMin = 0; int pomCnt = 0, swCnt = 0;
        while(!in.atEnd()){
            const QString line = in.readLine();
            if(line.startsWith("[Pomodoro]")){
                int mins = line.split(" ").last().replace("min"," ").trimmed().toInt();
                pomCnt++; pomMin += mins;
            } else if(line.startsWith("[Stopwatch]")){
                const QStringList parts = line.split(' ');
                bool ok=false; int sec=0; if(parts.size()>=2){ sec = parts.at(1).toInt(&ok);} if(ok){ swMin += (sec+59)/60; }
                swCnt++;
            } else if(line.contains("专注时长：")){
                int idx = line.indexOf("专注时长："); if(idx>=0){ QString sub = line.mid(idx + QString("专注时长：").size()); int end = sub.indexOf("；"); QString num = end>=0 ? sub.left(end) : sub; bool ok=false; int m = num.trimmed().toInt(&ok); if(ok) genericMin += m; }
            }
        }

        if(inWeek){
            s.pomodoroCountWeek += pomCnt; s.stopwatchEventsWeek += swCnt;
            s.pomodoroMinutesWeek += pomMin; s.stopwatchMinutesWeek += swMin;
            s.totalFocusMinutesWeek += (pomMin + swMin) > 0 ? (pomMin + swMin) : genericMin;
        }
        if(inMonth){
            s.pomodoroCountMonth += pomCnt; s.stopwatchEventsMonth += swCnt;
            s.pomodoroMinutesMonth += pomMin; s.stopwatchMinutesMonth += swMin;
            s.totalFocusMinutesMonth += (pomMin + swMin) > 0 ? (pomMin + swMin) : genericMin;
        }
    }
    return s;
}
