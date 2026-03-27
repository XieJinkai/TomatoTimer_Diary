#include "StatsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include "../services/Session.h"
#include "../services/StatsService.h"
#include "../services/AccountingStatsService.h"
#include "../ui/charts/PieChartWidget.h"
#include "../ui/charts/BarChartWidget.h"

StatsPage::StatsPage(QWidget* parent): QWidget(parent){ setupUi(); }

static AccountingRange rangeFromIndex(int index){
    if(index == 0) return AccountingRange::Day;
    if(index == 1) return AccountingRange::Week;
    if(index == 2) return AccountingRange::Month;
    return AccountingRange::Year;
}

static QString recordLine(const AccountingRecord& r){
    return QString("%1 | %2 | %3元 | %4").arg(r.time.toString("yyyy-MM-dd HH:mm")).arg(r.item).arg(QString::number(r.amount, 'f', 2)).arg(r.type);
}

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

    auto* consumeBox = new QVBoxLayout();
    auto* consumeTop = new QHBoxLayout();
    consumeTop->addWidget(new QLabel("消费统计", this));
    consumeTop->addStretch();
    consumeTop->addWidget(new QLabel("时间范围：", this));
    cmbConsumeRange_ = new QComboBox(this);
    cmbConsumeRange_->addItems({ "日", "周", "月", "年" });
    btnConsumeToggle_ = new QPushButton("切换图表", this);
    consumeTop->addWidget(cmbConsumeRange_);
    consumeTop->addWidget(btnConsumeToggle_);
    consumeBox->addLayout(consumeTop);

    stackConsume_ = new QStackedWidget(this);
    pieConsume_ = new PieChartWidget(this);
    barConsume_ = new BarChartWidget(this);
    stackConsume_->addWidget(pieConsume_);
    stackConsume_->addWidget(barConsume_);
    consumeBox->addWidget(stackConsume_);

    auto* sumRow = new QHBoxLayout();
    lblConsumeTotal_ = new QLabel(this);
    lblConsumeAvg_ = new QLabel(this);
    lblConsumeMax_ = new QLabel(this);
    lblConsumeMin_ = new QLabel(this);
    sumRow->addWidget(lblConsumeTotal_);
    sumRow->addWidget(lblConsumeAvg_);
    sumRow->addWidget(lblConsumeMax_);
    sumRow->addWidget(lblConsumeMin_);
    sumRow->addStretch();
    consumeBox->addLayout(sumRow);

    lay->addLayout(consumeBox);
    lay->addStretch();

    connect(btnToggle_, &QPushButton::clicked, this, &StatsPage::toggle);
    connect(btnConsumeToggle_, &QPushButton::clicked, this, &StatsPage::toggleConsumeChart);
    connect(cmbConsumeRange_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &StatsPage::onConsumeRangeChanged);
    connect(pieConsume_, &PieChartWidget::sliceClicked, this, &StatsPage::onConsumePieClicked);
    connect(barConsume_, &BarChartWidget::barClicked, this, &StatsPage::onConsumeBarClicked);
    refresh();
}

void StatsPage::refresh(){
    if(!Session::instance().isLoggedIn()){
        pieWeek_->setSlices({"未登录"}, {1.0}); barWeek_->setBars({ {"未登录", 1.0, QColor("#FF4D4F")} }, "");
        pieMonth_->setSlices({"未登录"}, {1.0}); barMonth_->setBars({ {"未登录", 1.0, QColor("#FF4D4F")} }, "");
        refreshConsume();
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
    refreshConsume();
}

void StatsPage::toggle(){ int idx = stackWeek_->currentIndex(); int next = idx==0?1:0; stackWeek_->setCurrentIndex(next); stackMonth_->setCurrentIndex(next); }

void StatsPage::refreshConsume(){
    if(!Session::instance().isLoggedIn()){
        pieConsume_->setSlices({"未登录"}, {1.0});
        barConsume_->setBars({ {"未登录", 1.0, QColor("#FF4D4F")} }, "");
        lblConsumeTotal_->setText("总消费：0.00元");
        lblConsumeAvg_->setText("平均消费：0.00元");
        lblConsumeMax_->setText("最高消费：无");
        lblConsumeMin_->setText("最低消费：无");
        return;
    }
    const QString user = Session::instance().username();
    const AccountingRange range = rangeFromIndex(cmbConsumeRange_->currentIndex());
    const auto sums = AccountingStatsService::sumByType(user, range);
    QStringList labels;
    QVector<double> values;
    for(auto it = sums.constBegin(); it != sums.constEnd(); ++it){
        labels.append(it.key());
        values.append(it.value());
    }
    if(labels.isEmpty()){
        pieConsume_->setSlices({"暂无数据"}, {1.0});
        barConsume_->setBars({ {"暂无数据", 1.0, QColor("#FA8C16")} }, "消费金额(元)");
    } else {
        pieConsume_->setSlices(labels, values);
        QList<BarItem> bars; QList<QColor> colors{ QColor("#FF4D4F"), QColor("#1890FF"), QColor("#52C41A"), QColor("#FA8C16"), QColor("#722ED1"), QColor("#13C2C2") };
        for(int i=0;i<labels.size();++i){ bars.append({ labels[i], values[i], colors[i % colors.size()] }); }
        barConsume_->setBars(bars, "消费金额(元)");
    }
    const auto summary = AccountingStatsService::summarize(user, range);
    lblConsumeTotal_->setText(QString("总消费：%1元").arg(QString::number(summary.total, 'f', 2)));
    lblConsumeAvg_->setText(QString("平均消费：%1元").arg(QString::number(summary.average, 'f', 2)));
    if(summary.hasRecord){
        lblConsumeMax_->setText(QString("最高消费：%1 | %2 | %3元 | %4").arg(summary.maxRecord.time.toString("yyyy-MM-dd")).arg(summary.maxRecord.item).arg(QString::number(summary.maxRecord.amount, 'f', 2)).arg(summary.maxRecord.type));
        lblConsumeMin_->setText(QString("最低消费：%1 | %2 | %3元 | %4").arg(summary.minRecord.time.toString("yyyy-MM-dd")).arg(summary.minRecord.item).arg(QString::number(summary.minRecord.amount, 'f', 2)).arg(summary.minRecord.type));
    } else {
        lblConsumeMax_->setText("最高消费：无");
        lblConsumeMin_->setText("最低消费：无");
    }
}

void StatsPage::toggleConsumeChart(){
    int idx = stackConsume_->currentIndex();
    int next = idx==0?1:0;
    stackConsume_->setCurrentIndex(next);
}

void StatsPage::onConsumeRangeChanged(int){
    refreshConsume();
}

void StatsPage::onConsumePieClicked(const QString& label, double){
    showConsumeDetail(label);
}

void StatsPage::onConsumeBarClicked(const QString& label, double){
    showConsumeDetail(label);
}

void StatsPage::showConsumeDetail(const QString& type){
    if(!Session::instance().isLoggedIn()){
        QMessageBox::information(this, "详情", "请先登录");
        return;
    }
    const QString user = Session::instance().username();
    const AccountingRange range = rangeFromIndex(cmbConsumeRange_->currentIndex());
    const auto records = AccountingStatsService::recordsByType(user, range, type);
    if(records.isEmpty()){
        QMessageBox::information(this, "详情", "暂无记录");
        return;
    }
    QStringList lines;
    int shown = 0;
    for(const auto& r : records){
        lines.append(recordLine(r));
        shown++;
        if(shown >= 50) break;
    }
    QString text = lines.join("\n");
    if(records.size() > shown){
        text += QString("\n共%1条，仅展示前%2条").arg(records.size()).arg(shown);
    }
    QMessageBox::information(this, QString("消费类型：%1").arg(type), text);
}

void StatsPage::showEvent(QShowEvent* event){
    QWidget::showEvent(event);
    refresh();
}
