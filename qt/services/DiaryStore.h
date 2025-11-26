#pragma once
#include <QString>
#include <QDate>
#include "DataStore.h"

class DiaryStore {
public:
    // 获取用户指定日期的日记文件路径
    static QString entryPath(const QString& user, const QDate& date){
        return DataStore::userDir(user) + QDir::separator() + date.toString("yyyy-MM-dd") + ".txt";
    }

    // 读取指定日期的日记内容
    static QString load(const QString& user, const QDate& date){
        return DataStore::readAll(entryPath(user, date));
    }

    // 保存指定日期的日记内容
    static bool save(const QString& user, const QDate& date, const QString& content){
        return DataStore::writeAll(entryPath(user, date), content);
    }

    // 追加一行到指定日期的日记
    static void appendLine(const QString& user, const QDate& date, const QString& line){
        DataStore::appendLine(entryPath(user, date), line);
    }
};
