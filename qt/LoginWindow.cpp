#include "LoginWindow.h"
#include "MainWindow.h"
#include "pages/LoginRegisterPage.h"

LoginWindow::LoginWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setWindowTitle("登录/注册");
    resize(640, 420);
}

void LoginWindow::setupUi(){
    page_ = new LoginRegisterPage(this);
    setCentralWidget(page_);
    connect(page_, &LoginRegisterPage::loggedIn, this, [this]{
        emit loginSucceeded();
        auto* mainWindow = new MainWindow();
        mainWindow->show();
        close();
    });
}
