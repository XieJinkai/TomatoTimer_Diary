#pragma once
#include <QMainWindow>
class LoginRegisterPage;

class LoginWindow : public QMainWindow {
    Q_OBJECT
public:
    // 创建登录主窗口
    explicit LoginWindow(QWidget* parent=nullptr);
signals:
    // 登录成功后发出，进入主界面
    void loginSucceeded();
private:
    LoginRegisterPage* page_{};
    // 初始化 UI
    void setupUi();
};
