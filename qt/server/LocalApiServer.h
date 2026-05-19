#pragma once

#include <QByteArray>
#include <QHash>
#include <QSqlDatabase>
#include <QString>
#include <QTcpServer>

class QTcpSocket;

class LocalApiServer : public QTcpServer {
    Q_OBJECT

public:
    explicit LocalApiServer(QObject* parent=nullptr);
    ~LocalApiServer() override;

    bool initializeDatabase(const QString& path);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QByteArray handleRequest(const QString& method, const QString& rawPath, const QByteArray& body);
    QByteArray registerUser(const QByteArray& body);
    QByteArray loginUser(const QByteArray& body);
    QByteArray uploadFile(int userId, const QByteArray& body);
    QByteArray listFiles(int userId);
    QByteArray downloadFile(int userId, const QString& filename);
    QByteArray deleteFile(int userId, const QString& filename);

    QString connectionName_;
    QSqlDatabase db_;
    QHash<QTcpSocket*, QByteArray> buffers_;
};
