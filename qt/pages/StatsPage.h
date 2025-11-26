#pragma once
#include <QWidget>
class QLabel; class QPushButton; class QStackedWidget; class PieChartWidget; class BarChartWidget;
class StatsPage : public QWidget {
    Q_OBJECT
public:
    // 创建数据统计页面
    explicit StatsPage(QWidget* parent=nullptr);
private:
    QPushButton* btnToggle_{};
    QStackedWidget* stackWeek_{}; PieChartWidget* pieWeek_{}; BarChartWidget* barWeek_{};
    QStackedWidget* stackMonth_{}; PieChartWidget* pieMonth_{}; BarChartWidget* barMonth_{};
    QLabel* lblWeekTotal_{}; QLabel* lblMonthTotal_{};
    // 初始化 UI
    void setupUi();
    // 刷新统计数据与图表
    void refresh();
    // 切换饼图/柱状图
    void toggle();
};
