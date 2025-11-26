#pragma once
#include <QLabel>
#include <QImage>
class ImageView : public QLabel {
    Q_OBJECT
public:
    // 构造预览视图
    explicit ImageView(QWidget* parent=nullptr);
    // 设置显示的图像
    void setImage(const QImage& img);
    // 清除已选点
    void clearPoints();
    // 获取选中的四点（图像坐标）
    QVector<QPointF> points() const;
protected:
    // 处理点选事件（最多四点）
    void mousePressEvent(QMouseEvent* ev) override;
    // 绘制点与连线
    void paintEvent(QPaintEvent* ev) override;
private:
    QImage img_{}; QVector<QPointF> pts_{}; QRect pixRect_{}; double scale_{1.0};
};
