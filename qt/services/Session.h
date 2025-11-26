#pragma once
#include <QString>

class Session {
public:
    static Session& instance(){ static Session s; return s; }
    bool isLoggedIn() const { return !username_.isEmpty(); }
    const QString& username() const { return username_; }
    void login(const QString& username){ username_ = username; }
    void logout(){ username_.clear(); }
private:
    QString username_{};
};