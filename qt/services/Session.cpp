#include "Session.h"

void Session::loginLocal(const QString& username){
    mode_ = LoginMode::Local;
    userId_ = -1;
    username_ = username;
    realName_.clear();
    school_.clear();
    major_.clear();
}

void Session::loginRemote(int userId, const QString& username, const QString& realName,
                          const QString& school, const QString& major){
    mode_ = LoginMode::Remote;
    userId_ = userId;
    username_ = username;
    realName_ = realName;
    school_ = school;
    major_ = major;
}

void Session::logout(){
    mode_ = LoginMode::None;
    userId_ = -1;
    username_.clear();
    realName_.clear();
    school_.clear();
    major_.clear();
}
