#pragma once
#include "AccountingStore.h"

#include <QMap>

struct AccountingSummary {
    bool hasRecord{false};
    double total{0.0};
    double average{0.0};
    AccountingRecord maxRecord{};
    AccountingRecord minRecord{};
};

namespace AccountingStatsService {
QMap<QString, double> sumByType(const QString& user, AccountingRange range);
AccountingSummary summarize(const QString& user, AccountingRange range);
QList<AccountingRecord> recordsByType(const QString& user, AccountingRange range, const QString& type);
QList<AccountingRecord> recordsInRange(const QString& user, AccountingRange range);
}
