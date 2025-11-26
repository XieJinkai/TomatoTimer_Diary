#pragma once
#include <QWidget>
class QLineEdit; class QPushButton; class QLabel;

class LoginRegisterPage : public QWidget {
    Q_OBJECT
public:
    // 创建登录/注册页面
    explicit LoginRegisterPage(QWidget* parent=nullptr);
signals:
    // 登录成功信号
    void loggedIn();
private:
    QLineEdit* userEdit_{}; QLineEdit* passEdit_{}; QLabel* info_{};
    QPushButton* btnRegister_{}; QPushButton* btnLogin_{}; QPushButton* btnLogout_{};
    // 初始化 UI
    void setupUi();
    // 注册新用户
    void onRegister();
    // 执行登录
    void onLogin();
    // 注销当前用户
    void onLogout();
};
