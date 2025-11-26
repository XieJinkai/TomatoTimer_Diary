#include "ImageView.h"
#include <QPainter>
#include <QMouseEvent>

ImageView::ImageView(QWidget* parent): QLabel(parent){ setMinimumSize(480,360); setAlignment(Qt::AlignCenter); }

void ImageView::setImage(const QImage& img){ img_ = img; QPixmap pm = QPixmap::fromImage(img_); QSize sz = pm.size(); QSize box = size(); QPixmap scaled = pm.scaled(box, Qt::KeepAspectRatio, Qt::SmoothTransformation); setPixmap(scaled); int w = scaled.width(), h = scaled.height(); int x = (box.width()-w)/2, y=(box.height()-h)/2; pixRect_ = QRect(x,y,w,h); scale_ = double(scaled.width())/double(img_.width()); update(); }

void ImageView::clearPoints(){ pts_.clear(); update(); }

QVector<QPointF> ImageView::points() const{ QVector<QPointF> res; for(const auto& p: pts_){ QPointF rel(p.x()-pixRect_.x(), p.y()-pixRect_.y()); QPointF imgp(rel.x()/scale_, rel.y()/scale_); res.append(imgp); } return res; }

void ImageView::mousePressEvent(QMouseEvent* ev){ if(!pixRect_.contains(ev->pos())) return; if(pts_.size()<4){ pts_.append(ev->pos()); update(); } }

void ImageView::paintEvent(QPaintEvent* ev){ QLabel::paintEvent(ev); if(img_.isNull()) return; QPainter p(this); p.setRenderHint(QPainter::Antialiasing); for(int i=0;i<pts_.size();++i){ QPointF pf = pts_[i]; QPoint pt = pf.toPoint(); p.setPen(QPen(Qt::red,2)); p.setBrush(QBrush(QColor(255,77,79))); p.drawEllipse(pt,5,5); if(i>0){ QPointF pprev = pts_[i-1]; p.drawLine(pprev.toPoint(), pt); } } }
