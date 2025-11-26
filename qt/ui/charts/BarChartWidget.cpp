#include "BarChartWidget.h"
#include <QPainter>

BarChartWidget::BarChartWidget(QWidget* parent): QWidget(parent){ setMinimumHeight(240); }

void BarChartWidget::setBars(const QList<BarItem>& items, const QString& axisLabel){ items_ = items; axisLabel_ = axisLabel; update(); }

void BarChartWidget::paintEvent(QPaintEvent*){
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    const int margin = 30; QRectF area(margin, margin, width()-2*margin, height()-2*margin);
    double maxv = 0; for(const auto& it: items_) maxv = std::max(maxv, it.value); if(maxv<=0) maxv=1;
    int n = items_.size(); double bw = area.width() / std::max(1,n) * 0.6; double gap = area.width() / std::max(1,n) * 0.4;
    double x = area.left(); int baseline = int(area.bottom());
    p.setPen(QPen(QColor("#999"))); p.drawLine(int(area.left()), baseline, int(area.right()), baseline);
    for(const auto& it : items_){
        double h = area.height() * (it.value / maxv);
        p.setBrush(it.color); p.setPen(Qt::NoPen);
        QRectF bar(x + gap*0.5, area.bottom()-h, bw, h);
        p.drawRoundedRect(bar, 3, 3);
        p.setPen(QPen(QColor("#666"))); p.drawText(int(bar.left()), baseline+16, it.label);
        p.drawText(int(bar.left()), int(bar.top())-6, QString::number(it.value));
        x += bw + gap;
    }
    if(!axisLabel_.isEmpty()){ p.setPen(QPen(QColor("#666"))); p.drawText(int(area.left()), int(area.top())-8, axisLabel_); }
}
