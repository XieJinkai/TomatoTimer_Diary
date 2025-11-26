#pragma once
#include <QWidget>
struct BarItem { QString label; double value; QColor color; };
class BarChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget* parent=nullptr);
    void setBars(const QList<BarItem>& items, const QString& axisLabel="");
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QList<BarItem> items_; QString axisLabel_{};
};
