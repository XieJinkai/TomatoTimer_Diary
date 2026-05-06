#include "SettingsSyncPage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include "../services/CloudSyncService.h"
#include "../services/Session.h"

SettingsSyncPage::SettingsSyncPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void SettingsSyncPage::setupUi(){
    sync_ = new CloudSyncService(this);

    auto* lay = new QVBoxLayout(this);
    auto* row = new QHBoxLayout();

    serverUrl_ = new QLineEdit(this);
    serverUrl_->setPlaceholderText("本地云服务器地址");
    serverUrl_->setText(sync_->serverUrl().toString());

    btnTest_ = new QPushButton("测试连接", this);
    btnUpload_ = new QPushButton("上传到云端", this);
    btnDownload_ = new QPushButton("从云端下载", this);
    info_ = new QLabel("启动 scripts/local_cloud_server.py 后，可通过 HTTP 模拟云同步。", this);
    info_->setWordWrap(true);

    row->addWidget(serverUrl_, 1);
    row->addWidget(btnTest_);
    row->addWidget(btnUpload_);
    row->addWidget(btnDownload_);
    lay->addLayout(row);
    lay->addWidget(info_);
    lay->addStretch();

    connect(btnTest_, &QPushButton::clicked, this, &SettingsSyncPage::testConnection);
    connect(btnUpload_, &QPushButton::clicked, this, &SettingsSyncPage::uploadNow);
    connect(btnDownload_, &QPushButton::clicked, this, &SettingsSyncPage::downloadNow);
    connect(sync_, &CloudSyncService::statusMessage, info_, &QLabel::setText);
}

bool SettingsSyncPage::applyServerUrl(){
    const QUrl url(serverUrl_->text().trimmed());
    if(!url.isValid() || url.scheme().isEmpty() || url.host().isEmpty()){
        info_->setText("请输入有效的服务器地址，例如 http://127.0.0.1:18080");
        return false;
    }
    sync_->setServerUrl(url);
    return true;
}

void SettingsSyncPage::testConnection(){
    if(!applyServerUrl()) return;
    sync_->testConnection();
}

void SettingsSyncPage::uploadNow(){
    if(!Session::instance().isLoggedIn()){
        info_->setText("请先登录");
        return;
    }
    if(Session::instance().isLocalMode()){
        info_->setText("本地模式不支持服务器同步，请注册或登录账号后使用同步功能。");
        return;
    }
    if(!applyServerUrl()) return;
    sync_->uploadUserFiles(Session::instance().username(), Session::instance().userId());
}

void SettingsSyncPage::downloadNow(){
    if(!Session::instance().isLoggedIn()){
        info_->setText("请先登录");
        return;
    }
    if(Session::instance().isLocalMode()){
        info_->setText("本地模式不支持服务器同步，请注册或登录账号后使用同步功能。");
        return;
    }
    if(!applyServerUrl()) return;
    sync_->downloadUserFiles(Session::instance().username(), Session::instance().userId());
}
