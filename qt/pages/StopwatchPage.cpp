#include "StopwatchPage.h"

#include <QDate>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

#include "../services/FocusRecordService.h"
#include "../services/Session.h"

StopwatchPage::StopwatchPage(QWidget* parent): QWidget(parent){ setupUi(); }

void StopwatchPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    display_ = new QLabel("00:00:00", this);
    display_->setStyleSheet("font-size:28px;font-weight:600");
    lay->addWidget(display_);
    remark_ = new QLineEdit(this);
    remark_->setPlaceholderText("备注（可选）");
    lay->addWidget(remark_);
    btnStart_ = new QPushButton("开始", this);
    btnStop_ = new QPushButton("结束并记录", this);
    btnReset_ = new QPushButton("重置", this);
    auto* row = new QHBoxLayout();
    row->addWidget(btnStart_);
    row->addWidget(btnStop_);
    row->addWidget(btnReset_);
    lay->addLayout(row);
    lay->addStretch();

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &StopwatchPage::tick);
    connect(btnStart_, &QPushButton::clicked, this, &StopwatchPage::start);
    connect(btnStop_, &QPushButton::clicked, this, &StopwatchPage::stop);
    connect(btnReset_, &QPushButton::clicked, this, &StopwatchPage::reset);
}

void StopwatchPage::start(){
    if(!timer_->isActive() && elapsedSec_ == 0){
        startedAt_ = QDateTime::currentDateTime();
    }
    timer_->start();
}

void StopwatchPage::stop(){
    timer_->stop();
    record();
}

void StopwatchPage::reset(){
    timer_->stop();
    elapsedSec_ = 0;
    startedAt_ = QDateTime();
    display_->setText("00:00:00");
}

void StopwatchPage::tick(){
    elapsedSec_++;
    display_->setText(QTime(0, 0, 0).addSecs(elapsedSec_).toString("hh:mm:ss"));
}

void StopwatchPage::record(){
    if(elapsedSec_ <= 0){
        QMessageBox::information(this, "正向计时", "计时为 0，未写入记录。");
        return;
    }
    if(!Session::instance().isLoggedIn()){
        QMessageBox::information(this, "正向计时", "请先登录后再记录。");
        return;
    }

    const QDateTime endedAt = QDateTime::currentDateTime();
    const QTime startTime = startedAt_.isValid() ? startedAt_.time() : endedAt.addSecs(-elapsedSec_).time();
    FocusRecordService::appendStopwatchRecord(
        Session::instance().username(),
        QDate::currentDate(),
        remark_->text(),
        startTime,
        endedAt.time(),
        elapsedSec_);

    elapsedSec_ = 0;
    startedAt_ = QDateTime();
    display_->setText("00:00:00");
}
