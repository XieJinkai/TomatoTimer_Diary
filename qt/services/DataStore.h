#pragma once
#include <QString>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

class DataStore {
public:
    static QString baseDir(){
        auto p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(p);
        return p;
    }

    static QString userDir(const QString& user){
        QString dir = baseDir() + QDir::separator() + user;
        QDir().mkpath(dir);
        return dir;
    }

    static bool appendLine(const QString& filePath, const QString& line){
        QFile f(filePath);
        if(!f.open(QIODevice::Append | QIODevice::Text)) return false;
        QTextStream out(&f);
        out << line << "\n";
        return true;
    }

    static QString readAll(const QString& filePath){
        QFile f(filePath);
        if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
        QTextStream in(&f);
        return in.readAll();
    }

    static bool writeAll(const QString& filePath, const QString& content){
        QFile f(filePath);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream out(&f);
        out << content;
        return true;
    }
};