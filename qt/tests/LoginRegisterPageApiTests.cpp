#include <QtTest/QtTest>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>

#include "../pages/LoginRegisterPage.h"

class LoginApiFakeServer : public QTcpServer {
    Q_OBJECT

public:
    explicit LoginApiFakeServer(QObject* parent=nullptr): QTcpServer(parent) {}

    QString url() const {
        return QString("http://127.0.0.1:%1").arg(serverPort());
    }

    QString lastPath;

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        auto* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, &QTcpSocket::readyRead, this, [this, socket](){
            const QByteArray data = socket->readAll();
            const QList<QByteArray> requestLine = data.left(data.indexOf('\n')).trimmed().split(' ');
            lastPath = QString::fromLatin1(requestLine.value(1));

            QByteArray body;
            int status = 200;
            QByteArray statusText = "OK";
            if(lastPath == "/health"){
                body = R"({"status":"ok"})";
            }else if(lastPath == "/api/login"){
                status = 400;
                statusText = "Bad Request";
                body = R"({"ok":false,"error":"用户名或密码错误"})";
            }else{
                status = 404;
                statusText = "Not Found";
                body = R"({"ok":false,"error":"接口不存在"})";
            }

            const QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + " " + statusText +
                "\r\nContent-Type: application/json; charset=utf-8"
                "\r\nContent-Length: " + QByteArray::number(body.size()) +
                "\r\nConnection: close\r\n\r\n" + body;
            socket->write(response);
            socket->disconnectFromHost();
        });
    }
};

class LoginRegisterPageApiTests : public QObject {
    Q_OBJECT

private slots:
    void loginBusinessErrorShowsServerMessage(){
        LoginApiFakeServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));

        LoginRegisterPage page;
        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));

        auto* serverUrl = page.findChild<QLineEdit*>("serverUrlEdit");
        QVERIFY(serverUrl != nullptr);
        serverUrl->setText(server.url());

        auto* loginUser = page.findChild<QLineEdit*>("loginUserEdit");
        auto* loginPass = page.findChild<QLineEdit*>("loginPassEdit");
        auto* loginButton = page.findChild<QPushButton*>("loginButton");
        auto* info = page.findChild<QLabel*>("loginInfoLabel");
        QVERIFY(loginUser != nullptr);
        QVERIFY(loginPass != nullptr);
        QVERIFY(loginButton != nullptr);
        QVERIFY(info != nullptr);

        loginUser->setText("missing_user");
        loginPass->setText("bad_password");
        QTest::mouseClick(loginButton, Qt::LeftButton);

        QTRY_COMPARE(info->text(), QString("用户名或密码错误"));
    }

    void testConnectionShowsServerReachable(){
        LoginApiFakeServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));

        LoginRegisterPage page;
        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));

        auto* serverUrl = page.findChild<QLineEdit*>("serverUrlEdit");
        auto* testButton = page.findChild<QPushButton*>("testConnectionButton");
        auto* info = page.findChild<QLabel*>("loginInfoLabel");
        QVERIFY(serverUrl != nullptr);
        QVERIFY(testButton != nullptr);
        QVERIFY(info != nullptr);

        serverUrl->setText(server.url());
        QTest::mouseClick(testButton, Qt::LeftButton);

        QTRY_COMPARE(info->text(), QString("本地 API 服务连接正常"));
    }
};

QTEST_MAIN(LoginRegisterPageApiTests)

#include "LoginRegisterPageApiTests.moc"
