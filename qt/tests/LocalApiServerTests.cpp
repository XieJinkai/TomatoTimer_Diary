#include <QtTest/QtTest>

#include <QEventLoop>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTemporaryDir>

#include "../server/LocalApiServer.h"

class LocalApiServerTests : public QObject {
    Q_OBJECT

private:
    struct Response {
        int status{0};
        QByteArray body;
    };

    static Response request(QNetworkAccessManager* manager,
                            const QUrl& url,
                            const QByteArray& method = QByteArrayLiteral("GET"),
                            const QByteArray& body = QByteArray()){
        QNetworkRequest networkRequest(url);
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = manager->sendCustomRequest(networkRequest, method, body);

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        Response response;
        response.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        response.body = reply->readAll();
        reply->deleteLater();
        return response;
    }

    static QUrl urlFor(const LocalApiServer& server, const QString& path){
        return QUrl(QString("http://127.0.0.1:%1%2").arg(server.serverPort()).arg(path));
    }

private slots:
    void deleteFileRemovesCloudFile(){
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        LocalApiServer server;
        QVERIFY(server.initializeDatabase(dir.filePath("cloud.db")));
        QVERIFY(server.listen(QHostAddress::LocalHost));

        QNetworkAccessManager manager;

        const QJsonObject registerPayload{
            {"username", "delete_user"},
            {"password", "secret"}
        };
        const Response registerResponse = request(
            &manager,
            urlFor(server, "/api/register"),
            QByteArrayLiteral("POST"),
            QJsonDocument(registerPayload).toJson(QJsonDocument::Compact)
        );
        QCOMPARE(registerResponse.status, 200);
        const int userId = QJsonDocument::fromJson(registerResponse.body).object().value("id").toInt();
        QVERIFY(userId > 0);

        const QJsonObject uploadPayload{
            {"filename", "img_01.png"},
            {"contentBase64", QString::fromLatin1(QByteArray("image-bytes").toBase64())}
        };
        QCOMPARE(request(&manager,
                         urlFor(server, QString("/api/users/%1/files/upload").arg(userId)),
                         QByteArrayLiteral("POST"),
                         QJsonDocument(uploadPayload).toJson(QJsonDocument::Compact)).status,
                 200);

        const Response deleteResponse = request(
            &manager,
            urlFor(server, QString("/api/users/%1/files/img_01.png").arg(userId)),
            QByteArrayLiteral("DELETE")
        );
        QCOMPARE(deleteResponse.status, 200);
        QCOMPARE(QJsonDocument::fromJson(deleteResponse.body).object().value("ok").toBool(), true);

        const Response listResponse = request(
            &manager,
            urlFor(server, QString("/api/users/%1/files").arg(userId))
        );
        QCOMPARE(listResponse.status, 200);
        QCOMPARE(QJsonDocument::fromJson(listResponse.body).array().size(), 0);
    }

    void deleteMissingFileReturnsNotFound(){
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        LocalApiServer server;
        QVERIFY(server.initializeDatabase(dir.filePath("cloud.db")));
        QVERIFY(server.listen(QHostAddress::LocalHost));

        QNetworkAccessManager manager;
        const Response response = request(
            &manager,
            urlFor(server, "/api/users/1/files/missing.png"),
            QByteArrayLiteral("DELETE")
        );

        QCOMPARE(response.status, 404);
    }
};

QTEST_MAIN(LocalApiServerTests)

#include "LocalApiServerTests.moc"
