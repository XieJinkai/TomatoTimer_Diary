#pragma once
#include <QDateTime>
#include <QList>
#include <QString>

struct AccountingRecord {
    QDateTime time{};
    QString item{};
    double amount{0.0};
    QString type{};
    QString note{};
};

enum class AccountingRange {
    Day,
    Week,
    Month,
    Year,
    All
};

class AccountingStore {
public:
    static QString recordsFile(const QString& user);
    static QList<AccountingRecord> loadAll(const QString& user);
    static bool saveAll(const QString& user, const QList<AccountingRecord>& records);
    static bool append(const QString& user, const AccountingRecord& record);
};
