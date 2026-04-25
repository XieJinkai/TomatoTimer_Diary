#include "AccountingStore.h"
#include "DataStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

namespace {

QJsonObject toJson(const AccountingRecord& record){
    QJsonObject obj;
    obj["time"] = record.time.toString(Qt::ISODate);
    obj["item"] = record.item;
    obj["amount"] = record.amount;
    obj["type"] = record.type;
    obj["note"] = record.note;
    return obj;
}

AccountingRecord fromJson(const QJsonObject& obj){
    AccountingRecord record;
    record.time = QDateTime::fromString(obj.value("time").toString(), Qt::ISODate);
    record.item = obj.value("item").toString();
    record.amount = obj.value("amount").toDouble();
    record.type = obj.value("type").toString();
    record.note = obj.value("note").toString();
    return record;
}

}

QString AccountingStore::recordsFile(const QString& user){
    return DataStore::userDir(user) + QDir::separator() + "accounting.json";
}

QList<AccountingRecord> AccountingStore::loadAll(const QString& user){
    QList<AccountingRecord> records;
    QFile file(recordsFile(user));
    if(!file.exists()){
        return records;
    }
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return records;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if(!doc.isArray()){
        return records;
    }

    for(const auto& value : doc.array()){
        if(value.isObject()){
            records.append(fromJson(value.toObject()));
        }
    }

    std::sort(records.begin(), records.end(), [](const AccountingRecord& left, const AccountingRecord& right){
        return left.time > right.time;
    });
    return records;
}

bool AccountingStore::saveAll(const QString& user, const QList<AccountingRecord>& records){
    QJsonArray array;
    for(const auto& record : records){
        array.append(toJson(record));
    }

    QFile file(recordsFile(user));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        return false;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return true;
}

bool AccountingStore::append(const QString& user, const AccountingRecord& record){
    auto records = loadAll(user);
    records.append(record);
    std::sort(records.begin(), records.end(), [](const AccountingRecord& left, const AccountingRecord& right){
        return left.time > right.time;
    });
    return saveAll(user, records);
}
