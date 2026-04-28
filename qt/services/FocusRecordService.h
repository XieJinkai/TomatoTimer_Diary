#pragma once

#include <QDate>
#include <QObject>
#include <QTime>
#include <QString>
#include <QStringList>

enum class FocusRecordType {
    Pomodoro,
    Stopwatch,
    Generic
};

struct FocusRecord {
    FocusRecordType type{FocusRecordType::Generic};
    QString task;
    QString status;
    QTime start;
    QTime end;
    int currentRound{0};
    int totalRounds{0};
    int plannedMin{0};
    int actualSec{0};
    int actualMin{0};
    bool legacy{false};
};

struct FocusSummary {
    int pomodoroCount{0};
    int stopwatchEvents{0};
    int totalFocusMinutes{0};
    int pomodoroMinutes{0};
    int stopwatchMinutes{0};
    int legacyOtherMinutes{0};
    QStringList detailLines;
};

struct DiaryContentParts {
    QStringList focusLines;
    QStringList mediaLines;
    QStringList bodyLines;
};

class FocusRecordEvents : public QObject {
    Q_OBJECT
public:
    explicit FocusRecordEvents(QObject* parent=nullptr) : QObject(parent) {}

signals:
    void recordAppended(const QString& user, const QDate& date);
};

class FocusRecordService {
public:
    static FocusRecordEvents* events();

    static QString buildPomodoroRecord(const QString& task,
                                       int currentRound,
                                       int totalRounds,
                                       const QString& status,
                                       const QTime& start,
                                       const QTime& end,
                                       int plannedMin,
                                       int actualSec);

    static QString buildStopwatchRecord(const QString& task,
                                        const QString& status,
                                        const QTime& start,
                                        const QTime& end,
                                        int actualSec);

    static bool parseRecordLine(const QString& line, FocusRecord* record);
    static bool parseFocusLine(const QString& line, FocusRecord* record);
    static FocusSummary summarizeContent(const QString& content);
    static DiaryContentParts splitDiaryContent(const QString& content);
    static QString renderDiaryText(const QString& content);
    static QString stripRenderedFocusBlock(const QString& content);
    static QString composeDiaryContent(const QStringList& focusLines,
                                       const QString& body,
                                       const QStringList& mediaLines);

    static bool appendPomodoroRecord(const QString& user,
                                     const QDate& date,
                                     const QString& task,
                                     int currentRound,
                                     int totalRounds,
                                     const QString& status,
                                     const QTime& start,
                                     const QTime& end,
                                     int plannedMin,
                                     int actualSec);

    static bool appendStopwatchRecord(const QString& user,
                                      const QDate& date,
                                      const QString& task,
                                      const QTime& start,
                                      const QTime& end,
                                      int actualSec);
};
