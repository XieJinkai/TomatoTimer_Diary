#pragma once
#include <QMainWindow>
class LoginRegisterPage;

class LoginWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent=nullptr);
signals:
    void loginSucceeded();
private:
    LoginRegisterPage* page_{};
    void setupUi();
};