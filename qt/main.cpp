#include <QApplication>
#include "MainWindow.h"
#include "LoginWindow.h"
#include "ui/Theme.h"
#include <QCoreApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trae");
    QCoreApplication::setApplicationName("TomatoTimerQt");
    app.setWindowIcon(QIcon(":/icon.png"));
    Theme::apply(Theme::load());
    LoginWindow login;
    QObject::connect(&login, &LoginWindow::loginSucceeded, [&login]{
        auto* w = new MainWindow();
        w->show();
        login.close();
    });
    login.show();
    return app.exec();
}
