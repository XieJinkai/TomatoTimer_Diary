#pragma once

#include <QWidget>

#include "../services/AccountingStore.h"
#include "../services/StatsService.h"

class BarChartWidget;
class PieChartWidget;
class QComboBox;
class QLabel;
class QPushButton;
class QShowEvent;
class QStackedWidget;

class StatsPage : public QWidget {
    Q_OBJECT
public:
    explicit StatsPage(QWidget* parent=nullptr);

private:
    QPushButton* btnFocusToggle_{};
    QComboBox* cmbFocusRange_{};
    QStackedWidget* stackFocusDay_{};
    PieChartWidget* pieFocusDay_{};
    BarChartWidget* barFocusDay_{};
    QLabel* lblFocusDayTotal_{};
    QStackedWidget* stackFocusRange_{};
    PieChartWidget* pieFocusRange_{};
    BarChartWidget* barFocusRange_{};
    QLabel* lblFocusRangeTotal_{};

    QPushButton* btnConsumeToggle_{};
    QComboBox* cmbConsumeRange_{};
    QStackedWidget* stackConsumeDay_{};
    PieChartWidget* pieConsumeDay_{};
    BarChartWidget* barConsumeDay_{};
    QLabel* lblConsumeDayTotal_{};
    QLabel* lblConsumeDayAvg_{};
    QLabel* lblConsumeDayMax_{};
    QLabel* lblConsumeDayMin_{};
    QStackedWidget* stackConsumeRange_{};
    PieChartWidget* pieConsumeRange_{};
    BarChartWidget* barConsumeRange_{};
    QLabel* lblConsumeRangeTotal_{};
    QLabel* lblConsumeRangeAvg_{};
    QLabel* lblConsumeRangeMax_{};
    QLabel* lblConsumeRangeMin_{};

    void setupUi();
    void refresh();
    void refreshFocus();
    void refreshConsume();
    void toggleFocusCharts();
    void toggleConsumeCharts();
    void refreshFocusBlock(QStackedWidget* stack, PieChartWidget* pie, BarChartWidget* bar, QLabel* totalLabel, StatsRange range, const QString& axisLabel);
    void refreshConsumeBlock(QStackedWidget* stack, PieChartWidget* pie, BarChartWidget* bar, QLabel* totalLabel, QLabel* avgLabel, QLabel* maxLabel, QLabel* minLabel, AccountingRange range);
    void showConsumeDetail(const QString& type, AccountingRange range);

protected:
    void showEvent(QShowEvent* event) override;
};
