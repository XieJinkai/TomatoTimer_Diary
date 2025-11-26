#include "StopwatchPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QLineEdit>
#include <QDate>
#include "../services/Session.h"
#include "../services/DiaryStore.h"
#include "../services/DataStore.h"
#include <QDir>

StopwatchPage::StopwatchPage(QWidget* parent): QWidget(parent){ setupUi(); }

void StopwatchPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    display_ = new QLabel("00:00:00", this); display_->setStyleSheet("font-size:28px;font-weight:600");
    lay->addWidget(display_);
    remark_ = new QLineEdit(this); remark_->setPlaceholderText("备注（可选）"); lay->addWidget(remark_);
    btnStart_ = new QPushButton("开始", this); btnStop_ = new QPushButton("结束并记录", this); btnReset_ = new QPushButton("重置", this);
    auto* row = new QHBoxLayout(); row->addWidget(btnStart_); row->addWidget(btnStop_); row->addWidget(btnReset_);
    lay->addLayout(row); lay->addStretch();
    timer_ = new QTimer(this); timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &StopwatchPage::tick);
    connect(btnStart_, &QPushButton::clicked, this, &StopwatchPage::start);
    connect(btnStop_, &QPushButton::clicked, this, &StopwatchPage::stop);
    connect(btnReset_, &QPushButton::clicked, this, &StopwatchPage::reset);
}

void StopwatchPage::start(){ timer_->start(); }
void StopwatchPage::stop(){ timer_->stop(); record(); }
void StopwatchPage::reset(){ timer_->stop(); elapsedSec_=0; display_->setText("00:00:00"); }
void StopwatchPage::tick(){ elapsedSec_++; int h=elapsedSec_/3600; int m=(elapsedSec_%3600)/60; int s=elapsedSec_%60; display_->setText(QString("%1:%2:%3").arg(h,2,10,'0').arg(m,2,10,'0').arg(s,2,10,'0')); }

void StopwatchPage::record(){
    if(!Session::instance().isLoggedIn()) return;
    const auto u = Session::instance().username();
    const auto d = QDate::currentDate();
    const QString rem = remark_->text();
    const int minutes = (elapsedSec_ + 59) / 60;
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
    QString formatted;
    if(rem.isEmpty()){
        formatted = QString("[专注时长：%1；当月次数：%2；]").arg(minutes).arg(monthCount+1);
    } else {
        formatted = QString("[专注事件：%1；专注时长：%2；当月次数：%3；]").arg(rem).arg(minutes).arg(monthCount+1);
    }
    DiaryStore::appendLine(u, d, formatted);

    QString todayContent = DiaryStore::load(u, d);
    auto lines = todayContent.split('\n');
    int totalMin = 0; QStringList kept;
    for(const auto& line: lines){
        if(line.startsWith("[本日专注时长：")) continue;
        if(line.startsWith("[Pomodoro]")){
            const QStringList parts = line.split(' ');
            bool ok=false; int m=0; if(parts.size()>=5){ m = parts.at(parts.size()-2).toInt(&ok);} if(ok) totalMin += m;
        } else if(line.startsWith("[Stopwatch]")){
            const QStringList parts = line.split(' ');
            bool ok=false; int s=0; if(parts.size()>=3){ s = parts.at(1).toInt(&ok);} if(ok) totalMin += s/60;
        } else if(line.contains("专注时长：")){
            int idx = line.indexOf("专注时长："); if(idx>=0){ QString sub = line.mid(idx + QString("专注时长：").size()); int end = sub.indexOf("；"); QString num = end>=0 ? sub.left(end) : sub; bool ok=false; int m = num.trimmed().toInt(&ok); if(ok) totalMin += m; }
        }
        kept.append(line);
    }
    kept.append(QString("[本日专注时长：%1]").arg(totalMin));
    DataStore::writeAll(DiaryStore::entryPath(u, d), kept.join("\n"));
}
