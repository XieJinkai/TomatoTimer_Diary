#pragma once
#include <QWidget>
class QLineEdit; class QPushButton; class QLabel;

class SettingsSyncPage : public QWidget {
    Q_OBJECT
public:
    // 创建设置/同步页面
    explicit SettingsSyncPage(QWidget* parent=nullptr);
private:
    QLineEdit* cloudDir_{}; QLabel* info_{}; QPushButton* btnChoose_{}; QPushButton* btnSync_{};
    // 初始化 UI
    void setupUi();
    // 选择云目录
    void chooseCloud();
    // 立刻同步到云目录
    void syncNow();
};
