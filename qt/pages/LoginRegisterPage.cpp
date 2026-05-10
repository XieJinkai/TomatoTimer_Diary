#include "LoginRegisterPage.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "../services/Session.h"

LoginRegisterPage::LoginRegisterPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void LoginRegisterPage::setupUi(){
    network_ = new QNetworkAccessManager(this);

    auto* layout = new QVBoxLayout(this);
    info_ = new QLabel("请选择本地模式，或使用账号登录。", this);
    info_->setObjectName("loginInfoLabel");
    info_->setWordWrap(true);

    serverUrlEdit_ = new QLineEdit(this);
    serverUrlEdit_->setObjectName("serverUrlEdit");
    serverUrlEdit_->setPlaceholderText("本地 API 服务地址");
    serverUrlEdit_->setText("http://127.0.0.1:18080");
    btnTestConnection_ = new QPushButton("测试连接", this);
    btnTestConnection_->setObjectName("testConnectionButton");

    auto* tabs = new QTabWidget(this);

    auto* localPage = new QWidget(this);
    auto* localLayout = new QVBoxLayout(localPage);
    localLayout->addWidget(new QLabel("不注册账号，直接使用固定本地账号 local_user。", localPage));
    btnLocal_ = new QPushButton("进入本地模式", localPage);
    localLayout->addWidget(btnLocal_);
    localLayout->addStretch();

    auto* loginPage = new QWidget(this);
    auto* loginForm = new QFormLayout(loginPage);
    loginUserEdit_ = new QLineEdit(loginPage);
    loginUserEdit_->setObjectName("loginUserEdit");
    loginPassEdit_ = new QLineEdit(loginPage);
    loginPassEdit_->setObjectName("loginPassEdit");
    loginPassEdit_->setEchoMode(QLineEdit::Password);
    btnLogin_ = new QPushButton("登录", loginPage);
    btnLogin_->setObjectName("loginButton");
    loginForm->addRow("用户名", loginUserEdit_);
    loginForm->addRow("密码", loginPassEdit_);
    loginForm->addRow(btnLogin_);

    auto* registerPage = new QWidget(this);
    auto* registerForm = new QFormLayout(registerPage);
    registerUserEdit_ = new QLineEdit(registerPage);
    registerPassEdit_ = new QLineEdit(registerPage);
    registerConfirmEdit_ = new QLineEdit(registerPage);
    realNameEdit_ = new QLineEdit(registerPage);
    genderEdit_ = new QLineEdit(registerPage);
    ageEdit_ = new QLineEdit(registerPage);
    phoneEdit_ = new QLineEdit(registerPage);
    emailEdit_ = new QLineEdit(registerPage);
    schoolEdit_ = new QLineEdit(registerPage);
    majorEdit_ = new QLineEdit(registerPage);
    registerPassEdit_->setEchoMode(QLineEdit::Password);
    registerConfirmEdit_->setEchoMode(QLineEdit::Password);
    btnRegister_ = new QPushButton("注册", registerPage);
    registerForm->addRow("用户名*", registerUserEdit_);
    registerForm->addRow("密码*", registerPassEdit_);
    registerForm->addRow("确认密码*", registerConfirmEdit_);
    registerForm->addRow("姓名", realNameEdit_);
    registerForm->addRow("性别", genderEdit_);
    registerForm->addRow("年龄", ageEdit_);
    registerForm->addRow("手机号", phoneEdit_);
    registerForm->addRow("邮箱", emailEdit_);
    registerForm->addRow("学校", schoolEdit_);
    registerForm->addRow("专业", majorEdit_);
    registerForm->addRow(btnRegister_);

    tabs->addTab(localPage, "本地模式");
    tabs->addTab(loginPage, "账号登录");
    tabs->addTab(registerPage, "账号注册");

    layout->addWidget(info_);
    auto* serverRow = new QHBoxLayout();
    serverRow->addWidget(serverUrlEdit_, 1);
    serverRow->addWidget(btnTestConnection_);
    layout->addLayout(serverRow);
    layout->addWidget(tabs);

    connect(btnLocal_, &QPushButton::clicked, this, &LoginRegisterPage::enterLocalMode);
    connect(btnTestConnection_, &QPushButton::clicked, this, &LoginRegisterPage::testConnection);
    connect(btnLogin_, &QPushButton::clicked, this, &LoginRegisterPage::onRemoteLogin);
    connect(btnRegister_, &QPushButton::clicked, this, &LoginRegisterPage::onRemoteRegister);
}

void LoginRegisterPage::enterLocalMode(){
    Session::instance().loginLocal("local_user");
    info_->setText("已进入本地模式：local_user");
    emit loggedIn();
}

void LoginRegisterPage::testConnection(){
    QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(serverUrlText() + "/health")));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ handleHealthReply(reply); });
}

void LoginRegisterPage::onRemoteLogin(){
    const QString username = loginUserEdit_->text().trimmed();
    const QString password = loginPassEdit_->text();
    if(username.isEmpty() || password.isEmpty()){
        info_->setText("请输入用户名和密码");
        return;
    }

    QJsonObject payload;
    payload.insert("username", username);
    payload.insert("password", password);

    QNetworkRequest request(QUrl(serverUrlText() + "/api/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = network_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ handleLoginReply(reply); });
}

void LoginRegisterPage::onRemoteRegister(){
    const QString username = registerUserEdit_->text().trimmed();
    const QString password = registerPassEdit_->text();
    const QString confirm = registerConfirmEdit_->text();
    if(username.isEmpty() || password.isEmpty() || confirm.isEmpty()){
        info_->setText("用户名、密码、确认密码为必填项");
        return;
    }
    if(password != confirm){
        info_->setText("两次输入的密码不一致");
        return;
    }

    QJsonObject payload;
    payload.insert("username", username);
    payload.insert("password", password);
    payload.insert("realName", realNameEdit_->text().trimmed());
    payload.insert("gender", genderEdit_->text().trimmed());
    payload.insert("age", ageEdit_->text().trimmed().toInt());
    payload.insert("phone", phoneEdit_->text().trimmed());
    payload.insert("email", emailEdit_->text().trimmed());
    payload.insert("school", schoolEdit_->text().trimmed());
    payload.insert("major", majorEdit_->text().trimmed());

    QNetworkRequest request(QUrl(serverUrlText() + "/api/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = network_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ handleRegisterReply(reply); });
}

void LoginRegisterPage::handleHealthReply(QNetworkReply* reply){
    const QString message = responseErrorMessage(reply, "无法连接本地 API 服务");
    if(reply->error() == QNetworkReply::NoError){
        info_->setText("本地 API 服务连接正常");
    }else{
        info_->setText(message);
    }
    reply->deleteLater();
}

void LoginRegisterPage::handleLoginReply(QNetworkReply* reply){
    const QByteArray body = reply->readAll();
    const QJsonObject root = QJsonDocument::fromJson(body).object();
    if(reply->error() != QNetworkReply::NoError && root.contains("error")){
        info_->setText(root.value("error").toString("登录失败"));
        reply->deleteLater();
        return;
    }
    if(reply->error() != QNetworkReply::NoError){
        info_->setText("登录失败：无法连接本地 API 服务");
        reply->deleteLater();
        return;
    }
    reply->deleteLater();
    if(!root.value("ok").toBool()){
        info_->setText(root.value("error").toString("登录失败"));
        return;
    }

    const QJsonObject user = root.value("user").toObject();
    Session::instance().loginRemote(
        user.value("id").toInt(),
        user.value("username").toString(),
        user.value("realName").toString(),
        user.value("school").toString(),
        user.value("major").toString()
    );
    info_->setText("已登录：" + Session::instance().username());
    emit loggedIn();
}

void LoginRegisterPage::handleRegisterReply(QNetworkReply* reply){
    const QByteArray body = reply->readAll();
    const QJsonObject root = QJsonDocument::fromJson(body).object();
    if(reply->error() != QNetworkReply::NoError && root.contains("error")){
        info_->setText(root.value("error").toString("注册失败"));
        reply->deleteLater();
        return;
    }
    if(reply->error() != QNetworkReply::NoError){
        info_->setText("注册失败：无法连接本地 API 服务");
        reply->deleteLater();
        return;
    }
    reply->deleteLater();
    if(root.value("ok").toBool()){
        info_->setText("注册成功，请切换到账号登录页登录");
    }else{
        info_->setText(root.value("error").toString("注册失败"));
    }
}

QString LoginRegisterPage::responseErrorMessage(QNetworkReply* reply, const QString& fallback) const {
    const QByteArray body = reply->peek(reply->bytesAvailable());
    const QJsonObject root = QJsonDocument::fromJson(body).object();
    if(root.contains("error")){
        return root.value("error").toString(fallback);
    }
    return fallback;
}

QString LoginRegisterPage::serverUrlText() const {
    QString url = serverUrlEdit_->text().trimmed();
    while(url.endsWith('/')){
        url.chop(1);
    }
    return url;
}
