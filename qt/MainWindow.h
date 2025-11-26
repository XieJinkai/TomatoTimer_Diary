#pragma once
#include <QMainWindow>
class QTabWidget;
class DiaryTabPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
private:
    QTabWidget* tabs_{};
    DiaryTabPage* diaryTab_{};
    void setupUi();
};
