#include "StatsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include "../services/Session.h"
#include "../services/StatsService.h"
#include "../ui/charts/PieChartWidget.h"
#include "../ui/charts/BarChartWidget.h"

StatsPage::StatsPage(QWidget* parent): QWidget(parent){ setupUi(); }

void StatsPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    auto* top = new QHBoxLayout();
    auto* title = new QLabel("数据统计", this);
    btnToggle_ = new QPushButton("切换图表", this);
    top->addWidget(title); top->addStretch(); top->addWidget(btnToggle_);
    lay->addLayout(top);

    auto* blocks = new QHBoxLayout();
    // Week block
    auto* weekBox = new QVBoxLayout();
    weekBox->addWidget(new QLabel("本周统计", this));
    stackWeek_ = new QStackedWidget(this);
    pieWeek_ = new PieChartWidget(this);
    barWeek_ = new BarChartWidget(this);
    stackWeek_->addWidget(pieWeek_);
    stackWeek_->addWidget(barWeek_);
    weekBox->addWidget(stackWeek_);
    lblWeekTotal_ = new QLabel(this); weekBox->addWidget(lblWeekTotal_);
    // Month block
    auto* monthBox = new QVBoxLayout();
    monthBox->addWidget(new QLabel("本月统计", this));
    stackMonth_ = new QStackedWidget(this);
    pieMonth_ = new PieChartWidget(this);
    barMonth_ = new BarChartWidget(this);
    stackMonth_->addWidget(pieMonth_);
    stackMonth_->addWidget(barMonth_);
    monthBox->addWidget(stackMonth_);
    lblMonthTotal_ = new QLabel(this); monthBox->addWidget(lblMonthTotal_);

    blocks->addLayout(weekBox);
    blocks->addLayout(monthBox);
    lay->addLayout(blocks);
    lay->addStretch();

    connect(btnToggle_, &QPushButton::clicked, this, &StatsPage::toggle);
    refresh();
}

void StatsPage::refresh(){
    if(!Session::instance().isLoggedIn()){
        pieWeek_->setSlices({"未登录"}, {1.0}); barWeek_->setBars({ {"未登录", 1.0, QColor("#FF4D4F")} }, "");
        pieMonth_->setSlices({"未登录"}, {1.0}); barMonth_->setBars({ {"未登录", 1.0, QColor("#FF4D4F")} }, "");
        return;
    }
    const auto s = StatsService::summarize(Session::instance().username());
    QStringList labelsW{ "番茄事件", "正向计时事件" };
    QVector<double> valuesW{ double(s.pomodoroMinutesWeek), double(s.stopwatchMinutesWeek) };
    pieWeek_->setSlices(labelsW, valuesW);
    QList<BarItem> barsW; QList<QColor> colors{ QColor("#FF4D4F"), QColor("#1890FF"), QColor("#52C41A"), QColor("#FA8C16"), QColor("#722ED1"), QColor("#13C2C2") };
    for(int i=0;i<labelsW.size();++i){ barsW.append({ labelsW[i], valuesW[i], colors[i % colors.size()] }); }
    barWeek_->setBars(barsW, "本周 时长(分钟)");
    lblWeekTotal_->setText(QString("专注时长：%1").arg(s.totalFocusMinutesWeek));

    QStringList labelsM{ "番茄事件", "正向计时事件" };
    QVector<double> valuesM{ double(s.pomodoroMinutesMonth), double(s.stopwatchMinutesMonth) };
    pieMonth_->setSlices(labelsM, valuesM);
    QList<BarItem> barsM; for(int i=0;i<labelsM.size();++i){ barsM.append({ labelsM[i], valuesM[i], colors[i % colors.size()] }); }
    barMonth_->setBars(barsM, "本月 时长(分钟)");
    lblMonthTotal_->setText(QString("专注分钟：%1").arg(s.totalFocusMinutesMonth));
}

void StatsPage::toggle(){ int idx = stackWeek_->currentIndex(); int next = idx==0?1:0; stackWeek_->setCurrentIndex(next); stackMonth_->setCurrentIndex(next); }
