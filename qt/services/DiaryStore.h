#pragma once
#include <QString>
#include <QDate>
#include "DataStore.h"

class DiaryStore {
public:
    static QString entryPath(const QString& user, const QDate& date){
        return DataStore::userDir(user) + QDir::separator() + date.toString("yyyy-MM-dd") + ".txt";
    }

    static QString load(const QString& user, const QDate& date){
        return DataStore::readAll(entryPath(user, date));
    }

    static bool save(const QString& user, const QDate& date, const QString& content){
        return DataStore::writeAll(entryPath(user, date), content);
    }

    static void appendLine(const QString& user, const QDate& date, const QString& line){
        DataStore::appendLine(entryPath(user, date), line);
    }
};