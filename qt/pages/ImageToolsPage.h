#pragma once
#include <QWidget>
class QLabel; class QPushButton; class ImageView;

class ImageToolsPage : public QWidget {
    Q_OBJECT
public:
    // 创建图片处理页面
    explicit ImageToolsPage(QWidget* parent=nullptr);
private:
    QLabel* info_{}; ImageView* view_{}; QLabel* ocrOut_{};
    QPushButton* btnOpen_{}; QPushButton* btnAdjust_{}; QPushButton* btnOcr_{}; QPushButton* btnSave_{};
    QString currentPath_{}; QImage currentImage_{};
    // 初始化 UI
    void setupUi();
    // 打开图片
    void openImage();
    // 透视调整（四点选择）
    void adjustPerspective();
    // 文字识别（Paddle/Tesseract）
    void recognizeText();
    // 保存当前图片
    void saveImage();
};
