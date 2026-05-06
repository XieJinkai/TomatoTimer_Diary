#include <QtTest/QtTest>

#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

#include "../LoginWindow.h"
#include "../MainWindow.h"
#include "../services/DataStore.h"
#include "../services/Session.h"

class LoginWindowFlowTests : public QObject {
    Q_OBJECT

private slots:
    void init(){
        Session::instance().logout();
    }

    void cleanup(){
        const auto topLevels = QApplication::topLevelWidgets();
        for(QWidget* widget : topLevels){
            if(widget){
                widget->close();
                if(qobject_cast<MainWindow*>(widget) != nullptr){
                    widget->deleteLater();
                }
            }
        }
        QCoreApplication::processEvents();
        Session::instance().logout();
    }

    void localModeOpensMainWindow(){
        QDir(DataStore::userDir("local_user")).removeRecursively();

        LoginWindow loginWindow;
        loginWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&loginWindow));

        const auto buttons = loginWindow.findChildren<QPushButton*>();
        QVERIFY(buttons.size() >= 1);

        QSignalSpy loginSpy(&loginWindow, &LoginWindow::loginSucceeded);

        QPushButton* localButton = nullptr;
        for(QPushButton* button : buttons){
            if(button->text().contains("本地")){
                localButton = button;
                break;
            }
        }
        QVERIFY(localButton != nullptr);

        QTest::mouseClick(localButton, Qt::LeftButton);
        QTRY_COMPARE(loginSpy.count(), 1);
        QCOMPARE(Session::instance().username(), QString("local_user"));

        bool foundMainWindow = false;
        for(QWidget* widget : QApplication::topLevelWidgets()){
            if(qobject_cast<MainWindow*>(widget) != nullptr){
                foundMainWindow = true;
                break;
            }
        }

        QVERIFY(foundMainWindow);
        QVERIFY(!loginWindow.isVisible());
    }
};

QTEST_MAIN(LoginWindowFlowTests)

#include "LoginWindowFlowTests.moc"
