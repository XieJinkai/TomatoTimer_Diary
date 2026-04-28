#include "FocusRecordService.h"

#include <QMap>
#include <QRegularExpression>

#include "DiaryStore.h"

namespace {

QString normalizedTask(const QString& task){
    const QString trimmed = task.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("未命名任务") : trimmed;
}

int roundedMinutes(int seconds){
    return seconds <= 0 ? 0 : (seconds + 59) / 60;
}

QString quoteValue(const QString& value){
    QString out;
    out.reserve(value.size() + 2);
    out.append('"');
    for(const QChar ch : value){
        if(ch == '\\' || ch == '"'){
            out.append('\\');
        }
        out.append(ch);
    }
    out.append('"');
    return out;
}

bool parseKeyValues(const QString& body, QMap<QString, QString>* values){
    int i = 0;
    while(i < body.size()){
        while(i < body.size() && body.at(i).isSpace()){
            ++i;
        }
        if(i >= body.size()){
            break;
        }

        const int keyStart = i;
        while(i < body.size() && body.at(i) != '='){
            ++i;
        }
        if(i >= body.size() || i == keyStart){
            return false;
        }
        const QString key = body.mid(keyStart, i - keyStart);
        ++i;
        if(i >= body.size()){
            return false;
        }

        QString value;
        if(body.at(i) == '"'){
            ++i;
            bool closed = false;
            while(i < body.size()){
                const QChar ch = body.at(i++);
                if(ch == '\\' && i < body.size()){
                    value.append(body.at(i++));
                } else if(ch == '"'){
                    closed = true;
                    break;
                } else {
                    value.append(ch);
                }
            }
            if(!closed){
                return false;
            }
        } else {
            const int valueStart = i;
            while(i < body.size() && !body.at(i).isSpace()){
                ++i;
            }
            value = body.mid(valueStart, i - valueStart);
        }
        values->insert(key, value);
    }
    return true;
}

bool parseIntValue(const QMap<QString, QString>& values, const QString& key, int* target){
    bool ok = false;
    const int value = values.value(key).toInt(&ok);
    if(!ok){
        return false;
    }
    *target = value;
    return true;
}

QString statusText(const FocusRecord& record){
    if(record.status == QStringLiteral("completed")){
        return QStringLiteral("完成");
    }
    if(record.status == QStringLiteral("ended-current")){
        return QStringLiteral("提前结束当前轮");
    }
    return QStringLiteral("记录");
}

QString detailLine(const FocusRecord& record){
    const QString task = normalizedTask(record.task);
    const QString duration = QStringLiteral("%1 分钟").arg(record.actualMin);
    const bool hasTime = record.start.isValid() && record.end.isValid();
    const QString timeRange = hasTime
        ? QStringLiteral("%1-%2 ").arg(record.start.toString("HH:mm"), record.end.toString("HH:mm"))
        : QString();

    if(record.type == FocusRecordType::Pomodoro){
        const QString roundText = record.currentRound > 0 && record.totalRounds > 0
            ? QStringLiteral(" %1/%2").arg(record.currentRound).arg(record.totalRounds)
            : QString();
        return QStringLiteral("%1番茄钟%2 %3：%4，%5")
            .arg(timeRange, roundText, statusText(record), task, duration);
    }
    if(record.type == FocusRecordType::Stopwatch){
        return QStringLiteral("%1正向计时：%2，%3").arg(timeRange, task, duration);
    }
    return QStringLiteral("%1专注：%2，%3").arg(timeRange, task, duration);
}

bool parsePomodoroLegacy(const QString& line, FocusRecord* record){
    if(!line.startsWith(QStringLiteral("[Pomodoro]"))){
        return false;
    }
    const QRegularExpression re(QStringLiteral("(\\d+)\\s*min"));
    const QRegularExpressionMatch match = re.match(line);
    if(!match.hasMatch()){
        return false;
    }
    record->type = FocusRecordType::Pomodoro;
    record->legacy = true;
    record->status = QStringLiteral("completed");
    record->actualMin = match.captured(1).toInt();
    record->actualSec = record->actualMin * 60;
    record->task = line.mid(QStringLiteral("[Pomodoro]").size()).left(match.capturedStart(1) - QStringLiteral("[Pomodoro]").size()).trimmed();
    return true;
}

bool parseStopwatchLegacy(const QString& line, FocusRecord* record){
    const QRegularExpression re(QStringLiteral("^\\[Stopwatch\\]\\s+(\\d+)(?:\\s+(.*))?$"));
    const QRegularExpressionMatch match = re.match(line);
    if(!match.hasMatch()){
        return false;
    }
    const int seconds = match.captured(1).toInt();
    record->type = FocusRecordType::Stopwatch;
    record->legacy = true;
    record->status = QStringLiteral("recorded");
    record->actualSec = seconds;
    record->actualMin = roundedMinutes(seconds);
    record->task = match.captured(2).trimmed();
    return true;
}

bool parseChineseLegacy(const QString& line, FocusRecord* record){
    QRegularExpression re(QStringLiteral("专注事件：([^；\\]]*)；专注时长：(\\d+)"));
    QRegularExpressionMatch match = re.match(line);
    if(!match.hasMatch()){
        re = QRegularExpression(QStringLiteral("专注时长：(\\d+)"));
        match = re.match(line);
    }
    if(!match.hasMatch()){
        re = QRegularExpression(QStringLiteral("涓撴敞浜嬩欢锛?([^锛漖]*)锛涙涓撴敞鏃堕暱锛?(\\d+)"));
        match = re.match(line);
    }
    if(!match.hasMatch()){
        re = QRegularExpression(QStringLiteral("涓撴敞鏃堕暱锛?(\\d+)"));
        match = re.match(line);
    }
    if(!match.hasMatch()){
        return false;
    }

    const bool hasTask = match.lastCapturedIndex() >= 2;
    const QString minutesText = hasTask ? match.captured(2) : match.captured(1);
    bool ok = false;
    const int minutes = minutesText.toInt(&ok);
    if(!ok){
        return false;
    }
    record->type = FocusRecordType::Generic;
    record->legacy = true;
    record->status = QStringLiteral("recorded");
    record->task = hasTask ? match.captured(1).trimmed() : QString();
    record->actualMin = minutes;
    record->actualSec = minutes * 60;
    return true;
}

bool isMediaLine(const QString& line){
    return line.startsWith(QStringLiteral("[Image]")) || line.startsWith(QStringLiteral("[Audio]"));
}

}

FocusRecordEvents* FocusRecordService::events(){
    static auto* eventSource = new FocusRecordEvents;
    return eventSource;
}

QString FocusRecordService::buildPomodoroRecord(const QString& task,
                                                int currentRound,
                                                int totalRounds,
                                                const QString& status,
                                                const QTime& start,
                                                const QTime& end,
                                                int plannedMin,
                                                int actualSec){
    const int actualMin = roundedMinutes(actualSec);
    return QStringLiteral("[FocusRecord type=pomodoro task=%1 round=%2/%3 status=%4 start=%5 end=%6 plannedMin=%7 actualSec=%8 actualMin=%9]")
        .arg(quoteValue(normalizedTask(task)))
        .arg(currentRound)
        .arg(totalRounds)
        .arg(status)
        .arg(start.toString("HH:mm"))
        .arg(end.toString("HH:mm"))
        .arg(plannedMin)
        .arg(actualSec)
        .arg(actualMin);
}

QString FocusRecordService::buildStopwatchRecord(const QString& task,
                                                 const QString& status,
                                                 const QTime& start,
                                                 const QTime& end,
                                                 int actualSec){
    const int actualMin = roundedMinutes(actualSec);
    return QStringLiteral("[FocusRecord type=stopwatch task=%1 status=%2 start=%3 end=%4 actualSec=%5 actualMin=%6]")
        .arg(quoteValue(normalizedTask(task)))
        .arg(status)
        .arg(start.toString("HH:mm"))
        .arg(end.toString("HH:mm"))
        .arg(actualSec)
        .arg(actualMin);
}

bool FocusRecordService::parseRecordLine(const QString& line, FocusRecord* record){
    if(!line.startsWith(QStringLiteral("[FocusRecord ")) || !line.endsWith(']') || record == nullptr){
        return false;
    }

    const QString body = line.mid(QStringLiteral("[FocusRecord ").size(), line.size() - QStringLiteral("[FocusRecord ").size() - 1);
    QMap<QString, QString> values;
    if(!parseKeyValues(body, &values)){
        return false;
    }

    FocusRecord parsed;
    const QString type = values.value(QStringLiteral("type"));
    if(type == QStringLiteral("pomodoro")){
        parsed.type = FocusRecordType::Pomodoro;
    } else if(type == QStringLiteral("stopwatch")){
        parsed.type = FocusRecordType::Stopwatch;
    } else {
        return false;
    }

    parsed.task = normalizedTask(values.value(QStringLiteral("task")));
    parsed.status = values.value(QStringLiteral("status"));
    parsed.start = QTime::fromString(values.value(QStringLiteral("start")), "HH:mm");
    parsed.end = QTime::fromString(values.value(QStringLiteral("end")), "HH:mm");
    if(parsed.status.isEmpty() || !parsed.start.isValid() || !parsed.end.isValid()){
        return false;
    }
    if(!parseIntValue(values, QStringLiteral("actualSec"), &parsed.actualSec) || parsed.actualSec <= 0){
        return false;
    }
    if(values.contains(QStringLiteral("actualMin"))){
        if(!parseIntValue(values, QStringLiteral("actualMin"), &parsed.actualMin)){
            return false;
        }
    } else {
        parsed.actualMin = roundedMinutes(parsed.actualSec);
    }

    if(parsed.type == FocusRecordType::Pomodoro){
        if(!parseIntValue(values, QStringLiteral("plannedMin"), &parsed.plannedMin)){
            return false;
        }
        const QStringList round = values.value(QStringLiteral("round")).split('/');
        if(round.size() != 2){
            return false;
        }
        bool currentOk = false;
        bool totalOk = false;
        parsed.currentRound = round.at(0).toInt(&currentOk);
        parsed.totalRounds = round.at(1).toInt(&totalOk);
        if(!currentOk || !totalOk || parsed.currentRound <= 0 || parsed.totalRounds <= 0){
            return false;
        }
    }

    *record = parsed;
    return true;
}

bool FocusRecordService::parseFocusLine(const QString& line, FocusRecord* record){
    if(parseRecordLine(line, record)){
        return true;
    }
    if(record == nullptr){
        return false;
    }
    FocusRecord parsed;
    if(parsePomodoroLegacy(line, &parsed) || parseStopwatchLegacy(line, &parsed) || parseChineseLegacy(line, &parsed)){
        *record = parsed;
        return true;
    }
    return false;
}

FocusSummary FocusRecordService::summarizeContent(const QString& content){
    FocusSummary summary;
    const auto lines = content.split('\n');
    for(const QString& line : lines){
        FocusRecord record;
        if(!parseFocusLine(line.trimmed(), &record)){
            continue;
        }
        switch(record.type){
        case FocusRecordType::Pomodoro:
            summary.pomodoroCount++;
            summary.pomodoroMinutes += record.actualMin;
            break;
        case FocusRecordType::Stopwatch:
            summary.stopwatchEvents++;
            summary.stopwatchMinutes += record.actualMin;
            break;
        case FocusRecordType::Generic:
            summary.legacyOtherMinutes += record.actualMin;
            break;
        }
        summary.totalFocusMinutes += record.actualMin;
        summary.detailLines.append(detailLine(record));
    }
    return summary;
}

DiaryContentParts FocusRecordService::splitDiaryContent(const QString& content){
    DiaryContentParts parts;
    const QString stripped = stripRenderedFocusBlock(content);
    const auto lines = stripped.split('\n');
    for(const QString& line : lines){
        const QString trimmed = line.trimmed();
        FocusRecord record;
        if(isMediaLine(trimmed)){
            parts.mediaLines.append(trimmed);
        } else if(parseFocusLine(trimmed, &record)){
            parts.focusLines.append(trimmed);
        } else if(trimmed == QStringLiteral("今日专注汇总") || trimmed == QStringLiteral("[专注明细]")){
            continue;
        } else {
            parts.bodyLines.append(line);
        }
    }
    return parts;
}

QString FocusRecordService::renderDiaryText(const QString& content){
    const DiaryContentParts parts = splitDiaryContent(content);
    const QString body = parts.bodyLines.join('\n').trimmed();
    if(parts.focusLines.isEmpty()){
        return body;
    }

    const FocusSummary summary = summarizeContent(parts.focusLines.join('\n'));
    QStringList rendered;
    rendered.append(QStringLiteral("今日专注汇总：共 %1 分钟；番茄钟 %2 次；正向计时 %3 次")
        .arg(summary.totalFocusMinutes)
        .arg(summary.pomodoroCount)
        .arg(summary.stopwatchEvents));
    rendered.append(QString());
    rendered.append(QStringLiteral("[专注明细]"));
    rendered.append(summary.detailLines);
    rendered.append(QString());
    rendered.append(QStringLiteral("----------------------------------"));
    if(!body.isEmpty()){
        rendered.append(body);
    }
    return rendered.join('\n');
}

QString FocusRecordService::stripRenderedFocusBlock(const QString& content){
    const QStringList lines = content.split('\n');
    bool hasGeneratedHeader = false;
    int separatorIndex = -1;
    for(int i = 0; i < lines.size(); ++i){
        const QString trimmed = lines.at(i).trimmed();
        if(i <= 2 && trimmed.startsWith(QStringLiteral("今日专注汇总"))){
            hasGeneratedHeader = true;
        }
        if(trimmed == QStringLiteral("----------------------------------")){
            separatorIndex = i;
            break;
        }
    }
    if(hasGeneratedHeader && separatorIndex >= 0){
        return lines.mid(separatorIndex + 1).join('\n');
    }
    return content;
}

QString FocusRecordService::composeDiaryContent(const QStringList& focusLines,
                                                const QString& body,
                                                const QStringList& mediaLines){
    QStringList output;
    for(const QString& line : focusLines){
        if(!line.trimmed().isEmpty()){
            output.append(line.trimmed());
        }
    }
    const QString strippedBody = stripRenderedFocusBlock(body).trimmed();
    if(!strippedBody.isEmpty()){
        output.append(strippedBody);
    }
    for(const QString& line : mediaLines){
        if(!line.trimmed().isEmpty()){
            output.append(line.trimmed());
        }
    }
    return output.join('\n');
}

bool FocusRecordService::appendPomodoroRecord(const QString& user,
                                              const QDate& date,
                                              const QString& task,
                                              int currentRound,
                                              int totalRounds,
                                              const QString& status,
                                              const QTime& start,
                                              const QTime& end,
                                              int plannedMin,
                                              int actualSec){
    if(actualSec <= 0){
        return false;
    }
    DiaryStore::appendLine(user, date, buildPomodoroRecord(task, currentRound, totalRounds, status, start, end, plannedMin, actualSec));
    emit events()->recordAppended(user, date);
    return true;
}

bool FocusRecordService::appendStopwatchRecord(const QString& user,
                                               const QDate& date,
                                               const QString& task,
                                               const QTime& start,
                                               const QTime& end,
                                               int actualSec){
    if(actualSec <= 0){
        return false;
    }
    DiaryStore::appendLine(user, date, buildStopwatchRecord(task, QStringLiteral("recorded"), start, end, actualSec));
    emit events()->recordAppended(user, date);
    return true;
}
