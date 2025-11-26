#pragma once
#include <QLabel>
#include <QImage>
class ImageView : public QLabel {
    Q_OBJECT
public:
    explicit ImageView(QWidget* parent=nullptr);
    void setImage(const QImage& img);
    void clearPoints();
    QVector<QPointF> points() const;
protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void paintEvent(QPaintEvent* ev) override;
private:
    QImage img_{}; QVector<QPointF> pts_{}; QRect pixRect_{}; double scale_{1.0};
};
