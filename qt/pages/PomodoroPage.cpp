#include "PomodoroPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDate>
#include <QDir>
#include "../services/Session.h"
#include "../services/DiaryStore.h"
#include "../services/DataStore.h"

PomodoroPage::PomodoroPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void PomodoroPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    taskName_ = new QLineEdit(this); taskName_->setPlaceholderText("任务名称");
    count_ = new QSpinBox(this); count_->setRange(1, 24); count_->setValue(4);
    focusMin_ = new QSpinBox(this); focusMin_->setRange(1, 120); focusMin_->setValue(25);
    restMin_ = new QSpinBox(this); restMin_->setRange(1, 60); restMin_->setValue(5);
    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel("番茄数:")); row->addWidget(count_);
    row->addWidget(new QLabel("专注(分钟):")); row->addWidget(focusMin_);
    row->addWidget(new QLabel("休息(分钟):")); row->addWidget(restMin_);
    lay->addWidget(taskName_); lay->addLayout(row);
    display_ = new QLabel("00:00", this); display_->setStyleSheet("font-size:28px;font-weight:600"); lay->addWidget(display_);
    btnStart_ = new QPushButton("开始", this);
    btnPause_ = new QPushButton("暂停", this);
    btnResume_ = new QPushButton("继续", this);
    btnEndEarly_ = new QPushButton("提前结束", this);
    auto* controls = new QHBoxLayout(); controls->addWidget(btnStart_); controls->addWidget(btnPause_); controls->addWidget(btnResume_); controls->addWidget(btnEndEarly_);
    lay->addLayout(controls); lay->addStretch();

    timer_ = new QTimer(this); timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &PomodoroPage::tick);
    connect(btnStart_, &QPushButton::clicked, this, &PomodoroPage::start);
    connect(btnPause_, &QPushButton::clicked, this, &PomodoroPage::pause);
    connect(btnResume_, &QPushButton::clicked, this, &PomodoroPage::resume);
    connect(btnEndEarly_, &QPushButton::clicked, this, &PomodoroPage::endEarly);
}

void PomodoroPage::start(){
    currentRound_ = 1; inRest_ = false;
    remainingSec_ = focusMin_->value()*60;
    timer_->start();
}

void PomodoroPage::pause(){ timer_->stop(); }
void PomodoroPage::resume(){ if(remainingSec_>0) timer_->start(); }

void PomodoroPage::tick(){
    if(remainingSec_>0) remainingSec_--; 
    int m = remainingSec_/60, s = remainingSec_%60;
    display_->setText(QString("%1:%2").arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')));
    if(remainingSec_==0){
        timer_->stop();
        if(!inRest_){
            // 专注完成一轮
            recordDone(focusMin_->value());
            inRest_ = true; remainingSec_ = restMin_->value()*60; timer_->start();
        } else {
            // 休息结束
            if(currentRound_ < count_->value()){
                currentRound_++;
                inRest_ = false; remainingSec_ = focusMin_->value()*60; timer_->start();
            } else {
                display_->setText("完成所有番茄");
            }
        }
    }
}

void PomodoroPage::recordDone(int minutes){
    if(!Session::instance().isLoggedIn()) return;
    const auto u = Session::instance().username();
    const auto d = QDate::currentDate();
    const QString name = taskName_->text().isEmpty()?"未命名任务":taskName_->text();

    QDir dir(DataStore::userDir(u));
    auto files = dir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Name);
    int monthCount = 0;
    for(const auto& fi : files){
        QDate fd = QDate::fromString(fi.baseName(), "yyyy-MM-dd");
        if(fd.year()==d.year() && fd.month()==d.month()){
            const QString content = DataStore::readAll(fi.filePath());
            const auto lines = content.split('\n');
            for(const auto& line: lines){
                if(line.contains("专注时长：")) monthCount++;
                else if(line.startsWith("[Pomodoro]")) monthCount++;
            }
        }
    }
    const QString formatted = QString("[专注事件：%1；专注时长：%2；当月次数：%3；]").arg(name).arg(minutes).arg(monthCount+1);
    DiaryStore::appendLine(u, d, formatted);

    QString todayContent = DiaryStore::load(u, d);
    auto lines = todayContent.split('\n');
    int totalMin = 0;
    QStringList kept;
    for(const auto& line: lines){
        if(line.startsWith("[本日专注时长：")) continue;
        if(line.startsWith("[Pomodoro]")){
            const QStringList parts = line.split(' ');
            bool ok=false; int m=0;
            if(parts.size()>=5){ m = parts.at(parts.size()-2).toInt(&ok); }
            if(ok) totalMin += m;
        } else if(line.contains("专注时长：")){
            // 提取“专注时长：X；”
            int idx = line.indexOf("专注时长：");
            if(idx>=0){
                QString sub = line.mid(idx + QString("专注时长：").size());
                int end = sub.indexOf("；");
                QString num = end>=0 ? sub.left(end) : sub;
                bool ok=false; int m = num.trimmed().toInt(&ok);
                if(ok) totalMin += m;
            }
        }
        kept.append(line);
    }
    kept.append(QString("[本日专注时长：%1]").arg(totalMin));
    DataStore::writeAll(DiaryStore::entryPath(u, d), kept.join("\n"));
}

void PomodoroPage::endEarly(){
    if(!timer_) return;
    if(!inRest_){
        int focusedSec = focusMin_->value()*60 - remainingSec_;
        if(focusedSec < 0) focusedSec = 0;
        int minutes = (focusedSec + 59) / 60;
        if(minutes > 0) recordDone(minutes);
        inRest_ = true; remainingSec_ = restMin_->value()*60; timer_->start();
    } else {
        if(currentRound_ < count_->value()){
            currentRound_++;
            inRest_ = false; remainingSec_ = focusMin_->value()*60; timer_->start();
        } else {
            timer_->stop(); display_->setText("完成所有番茄");
        }
    }
}
