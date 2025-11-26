#pragma once
#include <QWidget>
class QLineEdit; class QPushButton; class QLabel;

class LoginRegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginRegisterPage(QWidget* parent=nullptr);
signals:
    void loggedIn();
private:
    QLineEdit* userEdit_{}; QLineEdit* passEdit_{}; QLabel* info_{};
    QPushButton* btnRegister_{}; QPushButton* btnLogin_{}; QPushButton* btnLogout_{};
    void setupUi();
    void onRegister();
    void onLogin();
    void onLogout();
};