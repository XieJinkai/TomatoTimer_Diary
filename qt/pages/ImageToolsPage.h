#pragma once
#include <QWidget>
class QLabel; class QPushButton;

class ImageToolsPage : public QWidget {
    Q_OBJECT
public:
    explicit ImageToolsPage(QWidget* parent=nullptr);
private:
    QLabel* info_{}; QPushButton* btnOpen_{}; QPushButton* btnEnhance_{}; QString currentPath_{};
    void setupUi(); void openImage(); void enhance();
};