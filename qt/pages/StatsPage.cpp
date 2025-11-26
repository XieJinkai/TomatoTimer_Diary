#include "StatsPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include "../services/Session.h"
#include "../services/StatsService.h"

StatsPage::StatsPage(QWidget* parent): QWidget(parent){ setupUi(); }

void StatsPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    lbl_ = new QLabel("暂无数据", this);
    lay->addWidget(lbl_); lay->addStretch();
    refresh();
}

void StatsPage::refresh(){
    if(!Session::instance().isLoggedIn()){ lbl_->setText("请先登录"); return; }
    const auto s = StatsService::summarize(Session::instance().username());
    lbl_->setText(QString(
        "本周番茄：%1\n本周专注分钟：%2\n本周正向计时事件：%3\n\n本月番茄：%4\n本月专注分钟：%5\n本月正向计时事件：%6")
        .arg(s.pomodoroCountWeek).arg(s.totalFocusMinutesWeek).arg(s.stopwatchEventsWeek)
        .arg(s.pomodoroCountMonth).arg(s.totalFocusMinutesMonth).arg(s.stopwatchEventsMonth));
}