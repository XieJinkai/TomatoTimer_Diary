#include "SettingsSyncPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QFileInfoList>
#include "../services/Session.h"
#include "../services/DataStore.h"

SettingsSyncPage::SettingsSyncPage(QWidget* parent): QWidget(parent){ setupUi(); }

void SettingsSyncPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    cloudDir_ = new QLineEdit(this); cloudDir_->setPlaceholderText("选择云目录（本地模拟）");
    btnChoose_ = new QPushButton("选择目录", this);
    btnSync_ = new QPushButton("同步", this);
    info_ = new QLabel("选择目录后将用户数据复制到该目录。", this);
    auto* row = new QHBoxLayout(); row->addWidget(cloudDir_); row->addWidget(btnChoose_); row->addWidget(btnSync_);
    lay->addLayout(row); lay->addWidget(info_); lay->addStretch();

    connect(btnChoose_, &QPushButton::clicked, this, &SettingsSyncPage::chooseCloud);
    connect(btnSync_, &QPushButton::clicked, this, &SettingsSyncPage::syncNow);
}

void SettingsSyncPage::chooseCloud(){
    const QString dir = QFileDialog::getExistingDirectory(this, "选择同步目录");
    if(!dir.isEmpty()) cloudDir_->setText(dir);
}

void SettingsSyncPage::syncNow(){
    if(!Session::instance().isLoggedIn()){ info_->setText("请先登录"); return; }
    const QString dest = cloudDir_->text(); if(dest.isEmpty()){ info_->setText("请先选择目录"); return; }
    const QString src = DataStore::userDir(Session::instance().username());
    QDir srcDir(src); auto files = srcDir.entryInfoList(QDir::Files);
    int copied=0; for(const auto& fi:files){ QFile::copy(fi.filePath(), dest + QDir::separator() + fi.fileName()); copied++; }
    info_->setText(QString("已复制 %1 个文件到 %2").arg(copied).arg(dest));
}