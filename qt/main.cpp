#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

#include "LoginWindow.h"
#include "ui/Theme.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trae");
    QCoreApplication::setApplicationName("TomatoTimerQt");
    app.setWindowIcon(QIcon(":/icon.png"));
    Theme::apply(Theme::load());

    LoginWindow login;
    login.show();
    return app.exec();
}
