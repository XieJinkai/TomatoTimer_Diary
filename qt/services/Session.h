#pragma once
#include <QString>

class Session {
public:
    // 获取会话单例
    static Session& instance(){ static Session s; return s; }
    // 是否已登录
    bool isLoggedIn() const { return !username_.isEmpty(); }
    // 当前用户名
    const QString& username() const { return username_; }
    // 登录并设置用户名
    void login(const QString& username){ username_ = username; }
    // 注销并清空用户名
    void logout(){ username_.clear(); }
private:
    QString username_{};
};
