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

    void loginOpensMainWindow(){
        const QString username = "login_flow_test_user";
        QDir(DataStore::userDir(username)).removeRecursively();

        LoginWindow loginWindow;
        loginWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&loginWindow));

        const auto edits = loginWindow.findChildren<QLineEdit*>();
        QCOMPARE(edits.size(), 2);
        edits.at(0)->setText(username);
        edits.at(1)->setText("123456");

        const auto buttons = loginWindow.findChildren<QPushButton*>();
        QCOMPARE(buttons.size(), 2);

        QSignalSpy loginSpy(&loginWindow, &LoginWindow::loginSucceeded);

        QTest::mouseClick(buttons.at(0), Qt::LeftButton);
        QTest::mouseClick(buttons.at(1), Qt::LeftButton);
        QTRY_COMPARE(loginSpy.count(), 1);

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
