#include "AccountingStatsService.h"

#include <QDate>

namespace {

bool inRange(const QDateTime& time, AccountingRange range){
    if(!time.isValid()){
        return false;
    }
    if(range == AccountingRange::All){
        return true;
    }

    const QDate date = time.date();
    const QDate today = QDate::currentDate();
    if(range == AccountingRange::Day){
        return date == today;
    }
    if(range == AccountingRange::Week){
        const QDate weekStart = today.addDays(1 - today.dayOfWeek());
        return date >= weekStart && date <= today;
    }
    if(range == AccountingRange::Month){
        return date.year() == today.year() && date.month() == today.month();
    }
    return date.year() == today.year();
}

QList<AccountingRecord> filtered(const QString& user, AccountingRange range){
    QList<AccountingRecord> result;
    for(const auto& record : AccountingStore::loadAll(user)){
        if(inRange(record.time, range)){
            result.append(record);
        }
    }
    return result;
}

}

QMap<QString, double> AccountingStatsService::sumByType(const QString& user, AccountingRange range){
    QMap<QString, double> sums;
    for(const auto& record : filtered(user, range)){
        const QString key = record.type.trimmed().isEmpty() ? QStringLiteral("未分类") : record.type.trimmed();
        sums[key] += record.amount;
    }
    return sums;
}

AccountingSummary AccountingStatsService::summarize(const QString& user, AccountingRange range){
    AccountingSummary summary;
    const auto records = filtered(user, range);
    if(records.isEmpty()){
        return summary;
    }

    summary.hasRecord = true;
    summary.maxRecord = records.first();
    summary.minRecord = records.first();

    for(const auto& record : records){
        summary.total += record.amount;
        if(record.amount > summary.maxRecord.amount){
            summary.maxRecord = record;
        }
        if(record.amount < summary.minRecord.amount){
            summary.minRecord = record;
        }
    }

    summary.average = summary.total / records.size();
    return summary;
}

QList<AccountingRecord> AccountingStatsService::recordsByType(const QString& user, AccountingRange range, const QString& type){
    QList<AccountingRecord> result;
    for(const auto& record : filtered(user, range)){
        const QString normalized = record.type.trimmed().isEmpty() ? QStringLiteral("未分类") : record.type.trimmed();
        if(normalized == type){
            result.append(record);
        }
    }
    return result;
}

QList<AccountingRecord> AccountingStatsService::recordsInRange(const QString& user, AccountingRange range){
    return filtered(user, range);
}
