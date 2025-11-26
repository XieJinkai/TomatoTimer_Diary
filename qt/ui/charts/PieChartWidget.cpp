#include "PieChartWidget.h"
#include <QPainter>
#include <QPaintEvent>

PieChartWidget::PieChartWidget(QWidget* parent): QWidget(parent){ setMinimumHeight(240); }

void PieChartWidget::setSlices(const QStringList& labels, const QVector<double>& values){ labels_ = labels; values_ = values; update(); }

void PieChartWidget::paintEvent(QPaintEvent*){
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QRectF rect(20, 20, height()-40, height()-40);
    double total = 0; for(double v: values_) total += v; if(total <= 0) total = 1;
    QList<QColor> colors{ QColor("#FF4D4F"), QColor("#1890FF"), QColor("#52C41A"), QColor("#FA8C16") };
    double start = 0; for(int i=0;i<values_.size();++i){
        double span = 360.0 * (values_[i] / total);
        p.setBrush(colors[i % colors.size()]); p.setPen(Qt::NoPen);
        p.drawPie(rect, int(start*16), int(span*16));
        start += span;
    }
    int legendX = rect.right() + 30; int legendY = int(rect.top());
    for(int i=0;i<labels_.size() && i<values_.size(); ++i){
        p.setBrush(colors[i % colors.size()]); p.setPen(Qt::NoPen);
        p.drawRect(legendX, legendY, 14, 14); p.setPen(QPen(QColor("#666"))); p.setBrush(Qt::NoBrush);
        p.drawText(legendX+20, legendY+12, QString("%1: %2").arg(labels_[i]).arg(values_[i]));
        legendY += 22;
    }
}
