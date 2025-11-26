#pragma once
#include <QWidget>
struct BarItem { QString label; double value; QColor color; };
class BarChartWidget : public QWidget {
    Q_OBJECT
public:
    // 构造柱状图控件
    explicit BarChartWidget(QWidget* parent=nullptr);
    // 设置柱状图数据与纵轴标签
    void setBars(const QList<BarItem>& items, const QString& axisLabel="");
protected:
    // 绘制柱状图
    void paintEvent(QPaintEvent*) override;
private:
    QList<BarItem> items_; QString axisLabel_{};
};
