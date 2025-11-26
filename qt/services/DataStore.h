#pragma once
#include <QString>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

class DataStore {
public:
    // 应用数据根目录
    static QString baseDir(){
        auto p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(p);
        return p;
    }

    // 获取并创建用户数据目录
    static QString userDir(const QString& user){
        QString dir = baseDir() + QDir::separator() + user;
        QDir().mkpath(dir);
        return dir;
    }

    // 追加一行文本到文件
    static bool appendLine(const QString& filePath, const QString& line){
        QFile f(filePath);
        if(!f.open(QIODevice::Append | QIODevice::Text)) return false;
        QTextStream out(&f);
        out << line << "\n";
        return true;
    }

    // 读取整个文件内容
    static QString readAll(const QString& filePath){
        QFile f(filePath);
        if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
        QTextStream in(&f);
        return in.readAll();
    }

    // 写入整个文件内容
    static bool writeAll(const QString& filePath, const QString& content){
        QFile f(filePath);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream out(&f);
        out << content;
        return true;
    }
};
