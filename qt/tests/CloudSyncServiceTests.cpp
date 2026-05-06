#include <QtTest/QtTest>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>

#include "../services/CloudSyncService.h"
#include "../services/DataStore.h"

class FakeHttpServer : public QTcpServer {
    Q_OBJECT

public:
    struct Request {
        QString method;
        QString path;
        QByteArray body;
    };

    explicit FakeHttpServer(QObject* parent=nullptr): QTcpServer(parent) {}

    QString url() const {
        return QString("http://127.0.0.1:%1").arg(serverPort());
    }

    QList<Request> requests;

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        auto* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, &QTcpSocket::readyRead, this, [this, socket](){
            const QByteArray data = socket->readAll();
            const int headerEnd = data.indexOf("\r\n\r\n");
            if(headerEnd < 0){
                return;
            }

            const QByteArray header = data.left(headerEnd);
            const QList<QByteArray> lines = header.split('\n');
            const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');

            Request request;
            request.method = QString::fromLatin1(requestLine.value(0));
            request.path = QString::fromLatin1(requestLine.value(1));
            request.body = data.mid(headerEnd + 4);
            requests.append(request);

            QByteArray body;
            QByteArray contentType = "application/json";
            if(request.path == "/health"){
                body = R"({"status":"ok"})";
            }else if(request.path.endsWith("/files")){
                body = R"([{"filename":"cloud.txt","size":12,"modified":1}])";
            }else if(request.path.endsWith("/files/cloud.txt")){
                body = "from server\n";
                contentType = "application/octet-stream";
            }else{
                body = R"({"ok":true})";
            }

            QByteArray response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType +
                                  "\r\nContent-Length: " + QByteArray::number(body.size()) +
                                  "\r\nConnection: close\r\n\r\n" + body;
            socket->write(response);
            socket->disconnectFromHost();
        });
    }
};

class CloudSyncServiceTests : public QObject {
    Q_OBJECT

private slots:
    void testConnectionReportsSuccess(){
        FakeHttpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));

        CloudSyncService service;
        service.setServerUrl(QUrl(server.url()));
        QSignalSpy finished(&service, &CloudSyncService::operationFinished);

        service.testConnection();

        QTRY_COMPARE(finished.count(), 1);
        QCOMPARE(finished.takeFirst().at(0).toBool(), true);
        QCOMPARE(server.requests.first().path, QString("/health"));
    }

    void uploadUserFilesPostsFileContent(){
        const QString username = "cloud_sync_service_upload_test";
        const QString userDir = DataStore::userDir(username);
        QDir(userDir).removeRecursively();
        QDir().mkpath(userDir);
        QVERIFY(DataStore::writeAll(userDir + QDir::separator() + "diary.txt", "hello"));

        FakeHttpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));

        CloudSyncService service;
        service.setServerUrl(QUrl(server.url()));
        QSignalSpy finished(&service, &CloudSyncService::operationFinished);

        service.uploadUserFiles(username, 7);

        QTRY_COMPARE(finished.count(), 1);
        QCOMPARE(finished.takeFirst().at(0).toBool(), true);
        QCOMPARE(server.requests.first().method, QString("POST"));
        QVERIFY(server.requests.first().path.endsWith("/api/users/7/files/upload"));

        const auto body = QJsonDocument::fromJson(server.requests.first().body).object();
        QCOMPARE(body.value("filename").toString(), QString("diary.txt"));
        QVERIFY(!body.value("contentBase64").toString().isEmpty());

        QDir(userDir).removeRecursively();
    }

    void downloadUserFilesWritesCloudFilesLocally(){
        const QString username = "cloud_sync_service_download_test";
        const QString userDir = DataStore::userDir(username);
        QDir(userDir).removeRecursively();

        FakeHttpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));

        CloudSyncService service;
        service.setServerUrl(QUrl(server.url()));
        QSignalSpy finished(&service, &CloudSyncService::operationFinished);

        service.downloadUserFiles(username, 7);

        QTRY_COMPARE(finished.count(), 1);
        QCOMPARE(finished.takeFirst().at(0).toBool(), true);
        QCOMPARE(DataStore::readAll(userDir + QDir::separator() + "cloud.txt"), QString("from server\n"));

        QDir(userDir).removeRecursively();
    }
};

QTEST_MAIN(CloudSyncServiceTests)

#include "CloudSyncServiceTests.moc"
