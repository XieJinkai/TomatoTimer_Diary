#pragma once
#include <QWidget>
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    // 构造饼图控件
    explicit PieChartWidget(QWidget* parent=nullptr);
    // 设置饼图切片标签与数值
    void setSlices(const QStringList& labels, const QVector<double>& values);
signals:
    void sliceClicked(const QString& label, double value);
protected:
    // 绘制饼图
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    QStringList labels_; QVector<double> values_;
};
