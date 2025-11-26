#pragma once
#include <QWidget>
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget* parent=nullptr);
    void setSlices(const QStringList& labels, const QVector<double>& values);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QStringList labels_; QVector<double> values_;
};
