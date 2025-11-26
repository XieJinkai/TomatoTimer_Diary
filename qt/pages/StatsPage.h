#pragma once
#include <QWidget>
class QLabel; class QPushButton; class QStackedWidget; class PieChartWidget; class BarChartWidget;
class StatsPage : public QWidget {
    Q_OBJECT
public:
    explicit StatsPage(QWidget* parent=nullptr);
private:
    QPushButton* btnToggle_{};
    QStackedWidget* stackWeek_{}; PieChartWidget* pieWeek_{}; BarChartWidget* barWeek_{};
    QStackedWidget* stackMonth_{}; PieChartWidget* pieMonth_{}; BarChartWidget* barMonth_{};
    QLabel* lblWeekTotal_{}; QLabel* lblMonthTotal_{};
    void setupUi(); void refresh(); void toggle();
};
