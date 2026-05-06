#pragma once

#include <QString>

enum class LoginMode {
    None,
    Local,
    Remote
};

class Session {
public:
    static Session& instance(){ static Session s; return s; }

    bool isLoggedIn() const { return mode_ != LoginMode::None && !username_.isEmpty(); }
    bool isLocalMode() const { return mode_ == LoginMode::Local; }
    bool isRemoteMode() const { return mode_ == LoginMode::Remote; }

    LoginMode mode() const { return mode_; }
    int userId() const { return userId_; }
    const QString& username() const { return username_; }
    const QString& realName() const { return realName_; }
    const QString& school() const { return school_; }
    const QString& major() const { return major_; }

    void login(const QString& username){ loginLocal(username); }
    void loginLocal(const QString& username);
    void loginRemote(int userId, const QString& username, const QString& realName,
                     const QString& school, const QString& major);
    void logout();

private:
    LoginMode mode_{LoginMode::None};
    int userId_{-1};
    QString username_{};
    QString realName_{};
    QString school_{};
    QString major_{};
};
