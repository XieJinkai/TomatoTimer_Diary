#pragma once
#include <QMainWindow>
class QTabWidget;
class DiaryTabPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 创建应用主窗口
    explicit MainWindow(QWidget* parent=nullptr);
private:
    QTabWidget* tabs_{};
    DiaryTabPage* diaryTab_{};
    // 初始化 Tab 与工具栏
    void setupUi();
};
