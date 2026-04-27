#include "PomodoroPage.h"

#include <QDate>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

#include "../services/FocusRecordService.h"
#include "../services/Session.h"

PomodoroPage::PomodoroPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void PomodoroPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    taskName_ = new QLineEdit(this);
    taskName_->setPlaceholderText("任务名称");
    count_ = new QSpinBox(this);
    count_->setRange(1, 24);
    count_->setValue(4);
    focusMin_ = new QSpinBox(this);
    focusMin_->setRange(1, 120);
    focusMin_->setValue(25);
    restMin_ = new QSpinBox(this);
    restMin_->setRange(1, 60);
    restMin_->setValue(5);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel("番茄数"));
    row->addWidget(count_);
    row->addWidget(new QLabel("专注(分钟):"));
    row->addWidget(focusMin_);
    row->addWidget(new QLabel("休息(分钟):"));
    row->addWidget(restMin_);
    lay->addWidget(taskName_);
    lay->addLayout(row);

    phaseLabel_ = new QLabel("未开始", this);
    phaseLabel_->setStyleSheet("font-size:16px;font-weight:600");
    display_ = new QLabel("00:00", this);
    display_->setStyleSheet("font-size:28px;font-weight:600");
    lay->addWidget(phaseLabel_);
    lay->addWidget(display_);

    btnStart_ = new QPushButton("开始", this);
    btnPause_ = new QPushButton("暂停", this);
    btnResume_ = new QPushButton("继续", this);
    btnEndCurrent_ = new QPushButton("结束当前专注并记录", this);
    btnSkipRest_ = new QPushButton("跳过当前休息", this);
    btnEndPlan_ = new QPushButton("结束整个计划", this);
    btnReset_ = new QPushButton("重置", this);
    auto* controls = new QHBoxLayout();
    controls->addWidget(btnStart_);
    controls->addWidget(btnPause_);
    controls->addWidget(btnResume_);
    controls->addWidget(btnEndCurrent_);
    controls->addWidget(btnSkipRest_);
    controls->addWidget(btnEndPlan_);
    controls->addWidget(btnReset_);
    lay->addLayout(controls);
    lay->addStretch();

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &PomodoroPage::tick);
    connect(btnStart_, &QPushButton::clicked, this, &PomodoroPage::start);
    connect(btnPause_, &QPushButton::clicked, this, &PomodoroPage::pause);
    connect(btnResume_, &QPushButton::clicked, this, &PomodoroPage::resume);
    connect(btnEndCurrent_, &QPushButton::clicked, this, &PomodoroPage::endCurrentFocus);
    connect(btnSkipRest_, &QPushButton::clicked, this, &PomodoroPage::skipRest);
    connect(btnEndPlan_, &QPushButton::clicked, this, &PomodoroPage::endPlan);
    connect(btnReset_, &QPushButton::clicked, this, &PomodoroPage::reset);

    updateControls();
}

void PomodoroPage::start(){
    beginFocusRound(1);
    timer_->start();
}

void PomodoroPage::pause(){
    timer_->stop();
    updateControls();
}

void PomodoroPage::resume(){
    if(remainingSec_ > 0 && currentRound_ > 0){
        timer_->start();
    }
    updateControls();
}

void PomodoroPage::tick(){
    if(remainingSec_ > 0){
        remainingSec_--;
    }
    updateDisplay();

    if(remainingSec_ != 0){
        return;
    }

    timer_->stop();
    if(!inRest_){
        recordFocus(QStringLiteral("completed"), focusMin_->value() * 60);
        if(currentRound_ < count_->value()){
            beginRest();
            timer_->start();
        } else {
            finishPlan();
        }
    } else if(currentRound_ < count_->value()){
        beginFocusRound(currentRound_ + 1);
        timer_->start();
    } else {
        finishPlan();
    }
}

void PomodoroPage::endCurrentFocus(){
    if(currentRound_ <= 0 || inRest_){
        return;
    }
    const int focusedSec = qMax(0, focusMin_->value() * 60 - remainingSec_);
    if(focusedSec > 0){
        recordFocus(QStringLiteral("ended-current"), focusedSec);
    }
    timer_->stop();
    if(currentRound_ < count_->value()){
        beginRest();
        timer_->start();
    } else {
        finishPlan();
    }
}

void PomodoroPage::skipRest(){
    if(currentRound_ <= 0 || !inRest_){
        return;
    }
    timer_->stop();
    if(currentRound_ < count_->value()){
        beginFocusRound(currentRound_ + 1);
        timer_->start();
    } else {
        finishPlan();
    }
}

void PomodoroPage::endPlan(){
    if(currentRound_ <= 0){
        return;
    }

    if(!inRest_){
        const int focusedSec = qMax(0, focusMin_->value() * 60 - remainingSec_);
        if(focusedSec > 0){
            const auto choice = QMessageBox::question(
                this,
                "结束整个计划",
                "当前专注已有时长，是否记录这段专注后结束整个计划？",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);
            if(choice == QMessageBox::Cancel){
                return;
            }
            if(choice == QMessageBox::Yes){
                recordFocus(QStringLiteral("ended-current"), focusedSec);
            }
        }
    }
    finishPlan();
}

void PomodoroPage::reset(){
    timer_->stop();
    currentRound_ = 0;
    inRest_ = false;
    focusStartedAt_ = QDateTime();
    remainingSec_ = focusMin_->value() * 60;
    phaseLabel_->setText("未开始");
    updateDisplay();
    updateControls();
}

void PomodoroPage::beginFocusRound(int round){
    currentRound_ = round;
    inRest_ = false;
    remainingSec_ = focusMin_->value() * 60;
    focusStartedAt_ = QDateTime::currentDateTime();
    phaseLabel_->setText(QString("专注 %1/%2").arg(currentRound_).arg(count_->value()));
    updateDisplay();
    updateControls();
}

void PomodoroPage::beginRest(){
    inRest_ = true;
    remainingSec_ = restMin_->value() * 60;
    phaseLabel_->setText(QString("休息 %1/%2").arg(currentRound_).arg(count_->value()));
    updateDisplay();
    updateControls();
}

void PomodoroPage::finishPlan(){
    timer_->stop();
    remainingSec_ = 0;
    currentRound_ = 0;
    inRest_ = false;
    focusStartedAt_ = QDateTime();
    phaseLabel_->setText("计划已结束");
    display_->setText("00:00");
    updateControls();
}

void PomodoroPage::updateDisplay(){
    const int m = remainingSec_ / 60;
    const int s = remainingSec_ % 60;
    display_->setText(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    if(currentRound_ > 0){
        phaseLabel_->setText(QString("%1 %2/%3")
            .arg(inRest_ ? "休息" : "专注")
            .arg(currentRound_)
            .arg(count_->value()));
    }
}

void PomodoroPage::updateControls(){
    const bool running = currentRound_ > 0;
    btnPause_->setEnabled(running && timer_->isActive());
    btnResume_->setEnabled(running && !timer_->isActive() && remainingSec_ > 0);
    btnEndCurrent_->setEnabled(running && !inRest_);
    btnSkipRest_->setEnabled(running && inRest_);
    btnEndPlan_->setEnabled(running);
}

void PomodoroPage::recordFocus(const QString& status, int actualSec){
    if(actualSec <= 0 || !Session::instance().isLoggedIn()){
        return;
    }
    const QDateTime endedAt = QDateTime::currentDateTime();
    const QTime startTime = focusStartedAt_.isValid() ? focusStartedAt_.time() : endedAt.addSecs(-actualSec).time();
    FocusRecordService::appendPomodoroRecord(
        Session::instance().username(),
        QDate::currentDate(),
        taskName_->text(),
        currentRound_,
        count_->value(),
        status,
        startTime,
        endedAt.time(),
        focusMin_->value(),
        actualSec);
}
