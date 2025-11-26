#include "LoginRegisterPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include "../services/Session.h"
#include "../services/DataStore.h"

LoginRegisterPage::LoginRegisterPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void LoginRegisterPage::setupUi(){
    auto* layout = new QVBoxLayout(this);
    info_ = new QLabel("未登录", this);
    userEdit_ = new QLineEdit(this); userEdit_->setPlaceholderText("用户名");
    passEdit_ = new QLineEdit(this); passEdit_->setPlaceholderText("密码（本地演示，不加密）"); passEdit_->setEchoMode(QLineEdit::Password);
    btnRegister_ = new QPushButton("注册", this);
    btnLogin_ = new QPushButton("登录", this);
    btnLogout_ = new QPushButton("退出登录", this);

    layout->addWidget(info_);
    layout->addWidget(userEdit_);
    layout->addWidget(passEdit_);
    auto* row = new QHBoxLayout();
    row->addWidget(btnRegister_); row->addWidget(btnLogin_); row->addWidget(btnLogout_);
    layout->addLayout(row);
    layout->addStretch();

    connect(btnRegister_, &QPushButton::clicked, this, &LoginRegisterPage::onRegister);
    connect(btnLogin_, &QPushButton::clicked, this, &LoginRegisterPage::onLogin);
    connect(btnLogout_, &QPushButton::clicked, this, &LoginRegisterPage::onLogout);
}

void LoginRegisterPage::onRegister(){
    const auto user = userEdit_->text().trimmed();
    const auto pass = passEdit_->text();
    if(user.isEmpty()||pass.isEmpty()){ info_->setText("请输入用户名与密码"); return; }
    const QString dir = DataStore::userDir(user);
    const QString cred = dir + QDir::separator() + "credentials.txt";
    if(QFile::exists(cred)){ info_->setText("用户已存在"); return; }
    DataStore::writeAll(cred, pass);
    info_->setText("注册成功，请登录");
}

void LoginRegisterPage::onLogin(){
    const auto user = userEdit_->text().trimmed();
    const auto pass = passEdit_->text();
    if(user.isEmpty()||pass.isEmpty()){ info_->setText("请输入用户名与密码"); return; }
    const QString dir = DataStore::userDir(user);
    const QString cred = dir + QDir::separator() + "credentials.txt";
    if(!QFile::exists(cred)){ info_->setText("用户不存在，请先注册"); return; }
    const QString saved = DataStore::readAll(cred).trimmed();
    if(saved != pass){ info_->setText("密码错误"); return; }
    Session::instance().login(user);
    info_->setText("已登录：" + user);
    emit loggedIn();
}

void LoginRegisterPage::onLogout(){
    Session::instance().logout();
    info_->setText("未登录");
}