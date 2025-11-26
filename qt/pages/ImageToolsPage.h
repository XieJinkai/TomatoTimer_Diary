#pragma once
#include <QWidget>
class QLabel; class QPushButton; class ImageView;

class ImageToolsPage : public QWidget {
    Q_OBJECT
public:
    explicit ImageToolsPage(QWidget* parent=nullptr);
private:
    QLabel* info_{}; ImageView* view_{}; QLabel* ocrOut_{};
    QPushButton* btnOpen_{}; QPushButton* btnAdjust_{}; QPushButton* btnOcr_{}; QPushButton* btnSave_{};
    QString currentPath_{}; QImage currentImage_{};
    void setupUi(); void openImage(); void adjustPerspective(); void recognizeText(); void saveImage();
};
