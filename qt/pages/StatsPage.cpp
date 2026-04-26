#include "StatsPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "../services/AccountingStatsService.h"
#include "../services/Session.h"
#include "../ui/charts/BarChartWidget.h"
#include "../ui/charts/PieChartWidget.h"

namespace {

AccountingRange accountingRangeFromComboIndex(int index){
    switch(index){
    case 0: return AccountingRange::Week;
    case 1: return AccountingRange::Month;
    default: return AccountingRange::Year;
    }
}

QString accountingRecordLine(const AccountingRecord& record){
    return QString("%1 | %2 | %3元 | %4")
        .arg(record.time.toString("yyyy-MM-dd HH:mm"))
        .arg(record.item)
        .arg(QString::number(record.amount, 'f', 2))
        .arg(record.type.trimmed().isEmpty() ? QStringLiteral("未分类") : record.type);
}

QString rangeLabelText(AccountingRange range){
    switch(range){
    case AccountingRange::Day: return QStringLiteral("当日");
    case AccountingRange::Week: return QStringLiteral("本周");
    case AccountingRange::Month: return QStringLiteral("本月");
    case AccountingRange::Year: return QStringLiteral("本年");
    case AccountingRange::All: return QStringLiteral("全部");
    }
    return QStringLiteral("未知");
}

}

StatsPage::StatsPage(QWidget* parent): QWidget(parent){
    setupUi();
}

void StatsPage::setupUi(){
    auto* root = new QVBoxLayout(this);

    auto* titleRow = new QHBoxLayout();
    auto* title = new QLabel("数据统计", this);
    title->setStyleSheet("font-size:18px;font-weight:600;");
    titleRow->addWidget(title);
    titleRow->addStretch();
    root->addLayout(titleRow);

    auto* focusHeader = new QHBoxLayout();
    focusHeader->addWidget(new QLabel("专注统计", this));
    focusHeader->addStretch();
    focusHeader->addWidget(new QLabel("右侧范围", this));
    cmbFocusRange_ = new QComboBox(this);
    cmbFocusRange_->addItems({ "周", "月", "年" });
    btnFocusToggle_ = new QPushButton("切换图表", this);
    focusHeader->addWidget(cmbFocusRange_);
    focusHeader->addWidget(btnFocusToggle_);
    root->addLayout(focusHeader);

    auto* focusBlocks = new QHBoxLayout();

    auto* focusDayLayout = new QVBoxLayout();
    focusDayLayout->addWidget(new QLabel("当日统计", this));
    stackFocusDay_ = new QStackedWidget(this);
    pieFocusDay_ = new PieChartWidget(this);
    barFocusDay_ = new BarChartWidget(this);
    stackFocusDay_->addWidget(pieFocusDay_);
    stackFocusDay_->addWidget(barFocusDay_);
    focusDayLayout->addWidget(stackFocusDay_);
    lblFocusDayTotal_ = new QLabel(this);
    focusDayLayout->addWidget(lblFocusDayTotal_);

    auto* focusRangeLayout = new QVBoxLayout();
    focusRangeLayout->addWidget(new QLabel("时间范围统计", this));
    stackFocusRange_ = new QStackedWidget(this);
    pieFocusRange_ = new PieChartWidget(this);
    barFocusRange_ = new BarChartWidget(this);
    stackFocusRange_->addWidget(pieFocusRange_);
    stackFocusRange_->addWidget(barFocusRange_);
    focusRangeLayout->addWidget(stackFocusRange_);
    lblFocusRangeTotal_ = new QLabel(this);
    focusRangeLayout->addWidget(lblFocusRangeTotal_);

    focusBlocks->addLayout(focusDayLayout);
    focusBlocks->addLayout(focusRangeLayout);
    root->addLayout(focusBlocks);

    auto* consumeHeader = new QHBoxLayout();
    consumeHeader->addWidget(new QLabel("消费统计", this));
    consumeHeader->addStretch();
    consumeHeader->addWidget(new QLabel("右侧范围", this));
    cmbConsumeRange_ = new QComboBox(this);
    cmbConsumeRange_->addItems({ "周", "月", "年" });
    btnConsumeToggle_ = new QPushButton("切换图表", this);
    consumeHeader->addWidget(cmbConsumeRange_);
    consumeHeader->addWidget(btnConsumeToggle_);
    root->addLayout(consumeHeader);

    auto* consumeBlocks = new QHBoxLayout();

    auto* consumeDayLayout = new QVBoxLayout();
    consumeDayLayout->addWidget(new QLabel("当日消费", this));
    stackConsumeDay_ = new QStackedWidget(this);
    pieConsumeDay_ = new PieChartWidget(this);
    barConsumeDay_ = new BarChartWidget(this);
    stackConsumeDay_->addWidget(pieConsumeDay_);
    stackConsumeDay_->addWidget(barConsumeDay_);
    consumeDayLayout->addWidget(stackConsumeDay_);
    lblConsumeDayTotal_ = new QLabel(this);
    lblConsumeDayAvg_ = new QLabel(this);
    lblConsumeDayMax_ = new QLabel(this);
    lblConsumeDayMin_ = new QLabel(this);
    consumeDayLayout->addWidget(lblConsumeDayTotal_);
    consumeDayLayout->addWidget(lblConsumeDayAvg_);
    consumeDayLayout->addWidget(lblConsumeDayMax_);
    consumeDayLayout->addWidget(lblConsumeDayMin_);

    auto* consumeRangeLayout = new QVBoxLayout();
    consumeRangeLayout->addWidget(new QLabel("时间范围消费", this));
    stackConsumeRange_ = new QStackedWidget(this);
    pieConsumeRange_ = new PieChartWidget(this);
    barConsumeRange_ = new BarChartWidget(this);
    stackConsumeRange_->addWidget(pieConsumeRange_);
    stackConsumeRange_->addWidget(barConsumeRange_);
    consumeRangeLayout->addWidget(stackConsumeRange_);
    lblConsumeRangeTotal_ = new QLabel(this);
    lblConsumeRangeAvg_ = new QLabel(this);
    lblConsumeRangeMax_ = new QLabel(this);
    lblConsumeRangeMin_ = new QLabel(this);
    consumeRangeLayout->addWidget(lblConsumeRangeTotal_);
    consumeRangeLayout->addWidget(lblConsumeRangeAvg_);
    consumeRangeLayout->addWidget(lblConsumeRangeMax_);
    consumeRangeLayout->addWidget(lblConsumeRangeMin_);

    consumeBlocks->addLayout(consumeDayLayout);
    consumeBlocks->addLayout(consumeRangeLayout);
    root->addLayout(consumeBlocks);
    root->addStretch();

    connect(btnFocusToggle_, &QPushButton::clicked, this, &StatsPage::toggleFocusCharts);
    connect(btnConsumeToggle_, &QPushButton::clicked, this, &StatsPage::toggleConsumeCharts);
    connect(cmbFocusRange_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int){ refreshFocus(); });
    connect(cmbConsumeRange_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int){ refreshConsume(); });

    connect(pieConsumeDay_, &PieChartWidget::sliceClicked, this, [this](const QString& label, double value){ Q_UNUSED(value); showConsumeDetail(label, AccountingRange::Day); });
    connect(barConsumeDay_, &BarChartWidget::barClicked, this, [this](const QString& label, double value){ Q_UNUSED(value); showConsumeDetail(label, AccountingRange::Day); });
    connect(pieConsumeRange_, &PieChartWidget::sliceClicked, this, [this](const QString& label, double value){ Q_UNUSED(value); showConsumeDetail(label, accountingRangeFromComboIndex(cmbConsumeRange_->currentIndex())); });
    connect(barConsumeRange_, &BarChartWidget::barClicked, this, [this](const QString& label, double value){ Q_UNUSED(value); showConsumeDetail(label, accountingRangeFromComboIndex(cmbConsumeRange_->currentIndex())); });

    refresh();
}

void StatsPage::refresh(){
    refreshFocus();
    refreshConsume();
}

void StatsPage::refreshFocus(){
    if(!Session::instance().isLoggedIn()){
        pieFocusDay_->setSlices({ "未登录" }, { 1.0 });
        barFocusDay_->setBars({ { "未登录", 1.0, QColor("#FF4D4F") } }, "");
        lblFocusDayTotal_->setText("当日专注时长：0 分钟");

        pieFocusRange_->setSlices({ "未登录" }, { 1.0 });
        barFocusRange_->setBars({ { "未登录", 1.0, QColor("#FF4D4F") } }, "");
        lblFocusRangeTotal_->setText("时间范围专注时长：0 分钟");
        return;
    }

    refreshFocusBlock(stackFocusDay_, pieFocusDay_, barFocusDay_, lblFocusDayTotal_, StatsRange::Day, "当日专注时长（分钟）");

    StatsRange selectedRange = StatsRange::Week;
    switch(cmbFocusRange_->currentIndex()){
    case 0: selectedRange = StatsRange::Week; break;
    case 1: selectedRange = StatsRange::Month; break;
    default: selectedRange = StatsRange::Year; break;
    }
    refreshFocusBlock(stackFocusRange_, pieFocusRange_, barFocusRange_, lblFocusRangeTotal_, selectedRange, "范围专注时长（分钟）");
}

void StatsPage::refreshFocusBlock(QStackedWidget* stack, PieChartWidget* pie, BarChartWidget* bar, QLabel* totalLabel, StatsRange range, const QString& axisLabel){
    Q_UNUSED(stack);
    const auto summary = StatsService::summarize(Session::instance().username(), range);

    const QStringList labels{ "番茄钟", "正向计时" };
    const QVector<double> values{ double(summary.pomodoroMinutes), double(summary.stopwatchMinutes) };
    pie->setSlices(labels, values);

    const QList<QColor> colors{ QColor("#FF4D4F"), QColor("#1890FF") };
    QList<BarItem> bars;
    for(int i = 0; i < labels.size(); ++i){
        bars.append({ labels[i], values[i], colors[i % colors.size()] });
    }
    bar->setBars(bars, axisLabel);

    QString prefix = "当日";
    switch(range){
    case StatsRange::Day: prefix = "当日"; break;
    case StatsRange::Week: prefix = "本周"; break;
    case StatsRange::Month: prefix = "本月"; break;
    case StatsRange::Year: prefix = "本年"; break;
    }
    totalLabel->setText(QString("%1专注时长：%2 分钟 | 番茄次数：%3 | 计时次数：%4")
        .arg(prefix)
        .arg(summary.totalFocusMinutes)
        .arg(summary.pomodoroCount)
        .arg(summary.stopwatchEvents));
}

void StatsPage::refreshConsume(){
    if(!Session::instance().isLoggedIn()){
        refreshConsumeBlock(stackConsumeDay_, pieConsumeDay_, barConsumeDay_, lblConsumeDayTotal_, lblConsumeDayAvg_, lblConsumeDayMax_, lblConsumeDayMin_, AccountingRange::Day);
        refreshConsumeBlock(stackConsumeRange_, pieConsumeRange_, barConsumeRange_, lblConsumeRangeTotal_, lblConsumeRangeAvg_, lblConsumeRangeMax_, lblConsumeRangeMin_, AccountingRange::Week);
        return;
    }

    refreshConsumeBlock(stackConsumeDay_, pieConsumeDay_, barConsumeDay_, lblConsumeDayTotal_, lblConsumeDayAvg_, lblConsumeDayMax_, lblConsumeDayMin_, AccountingRange::Day);
    refreshConsumeBlock(stackConsumeRange_, pieConsumeRange_, barConsumeRange_, lblConsumeRangeTotal_, lblConsumeRangeAvg_, lblConsumeRangeMax_, lblConsumeRangeMin_, accountingRangeFromComboIndex(cmbConsumeRange_->currentIndex()));
}

void StatsPage::refreshConsumeBlock(QStackedWidget* stack, PieChartWidget* pie, BarChartWidget* bar, QLabel* totalLabel, QLabel* avgLabel, QLabel* maxLabel, QLabel* minLabel, AccountingRange range){
    Q_UNUSED(stack);
    if(!Session::instance().isLoggedIn()){
        pie->setSlices({ "未登录" }, { 1.0 });
        bar->setBars({ { "未登录", 1.0, QColor("#FF4D4F") } }, "");
        totalLabel->setText("总消费：0.00 元");
        avgLabel->setText("平均消费：0.00 元");
        maxLabel->setText("最高消费：无");
        minLabel->setText("最低消费：无");
        return;
    }

    const QString user = Session::instance().username();
    const auto sums = AccountingStatsService::sumByType(user, range);

    QStringList labels;
    QVector<double> values;
    for(auto it = sums.constBegin(); it != sums.constEnd(); ++it){
        labels.append(it.key());
        values.append(it.value());
    }

    if(labels.isEmpty()){
        pie->setSlices({ "暂无数据" }, { 1.0 });
        bar->setBars({ { "暂无数据", 1.0, QColor("#FA8C16") } }, "消费金额（元）");
    } else {
        pie->setSlices(labels, values);
        const QList<QColor> colors{ QColor("#FF4D4F"), QColor("#1890FF"), QColor("#52C41A"), QColor("#FA8C16"), QColor("#722ED1"), QColor("#13C2C2") };
        QList<BarItem> bars;
        for(int i = 0; i < labels.size(); ++i){
            bars.append({ labels[i], values[i], colors[i % colors.size()] });
        }
        bar->setBars(bars, "消费金额（元）");
    }

    const auto summary = AccountingStatsService::summarize(user, range);
    const QString prefix = rangeLabelText(range);
    totalLabel->setText(QString("%1总消费：%2 元").arg(prefix).arg(QString::number(summary.total, 'f', 2)));
    avgLabel->setText(QString("%1平均消费：%2 元").arg(prefix).arg(QString::number(summary.average, 'f', 2)));
    if(summary.hasRecord){
        maxLabel->setText(QString("%1最高消费：%2 | %3 | %4 元 | %5")
            .arg(prefix)
            .arg(summary.maxRecord.time.toString("yyyy-MM-dd"))
            .arg(summary.maxRecord.item)
            .arg(QString::number(summary.maxRecord.amount, 'f', 2))
            .arg(summary.maxRecord.type.trimmed().isEmpty() ? QStringLiteral("未分类") : summary.maxRecord.type));
        minLabel->setText(QString("%1最低消费：%2 | %3 | %4 元 | %5")
            .arg(prefix)
            .arg(summary.minRecord.time.toString("yyyy-MM-dd"))
            .arg(summary.minRecord.item)
            .arg(QString::number(summary.minRecord.amount, 'f', 2))
            .arg(summary.minRecord.type.trimmed().isEmpty() ? QStringLiteral("未分类") : summary.minRecord.type));
    } else {
        maxLabel->setText(QString("%1最高消费：无").arg(prefix));
        minLabel->setText(QString("%1最低消费：无").arg(prefix));
    }
}

void StatsPage::toggleFocusCharts(){
    const int next = stackFocusDay_->currentIndex() == 0 ? 1 : 0;
    stackFocusDay_->setCurrentIndex(next);
    stackFocusRange_->setCurrentIndex(next);
}

void StatsPage::toggleConsumeCharts(){
    const int next = stackConsumeDay_->currentIndex() == 0 ? 1 : 0;
    stackConsumeDay_->setCurrentIndex(next);
    stackConsumeRange_->setCurrentIndex(next);
}

void StatsPage::showConsumeDetail(const QString& type, AccountingRange range){
    if(!Session::instance().isLoggedIn()){
        QMessageBox::information(this, "详情", "请先登录");
        return;
    }

    const auto records = AccountingStatsService::recordsByType(Session::instance().username(), range, type);
    if(records.isEmpty()){
        QMessageBox::information(this, "详情", "暂无记录");
        return;
    }

    QStringList lines;
    int shown = 0;
    for(const auto& record : records){
        lines.append(accountingRecordLine(record));
        shown++;
        if(shown >= 50){
            break;
        }
    }
    QString text = lines.join("\n");
    if(records.size() > shown){
        text += QString("\n共 %1 条，仅展示前 %2 条").arg(records.size()).arg(shown);
    }
    QMessageBox::information(this, QString("%1消费分类：%2").arg(rangeLabelText(range)).arg(type), text);
}

void StatsPage::showEvent(QShowEvent* event){
    QWidget::showEvent(event);
    refresh();
}
