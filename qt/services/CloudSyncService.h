#pragma once

#include <QObject>
#include <QUrl>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkReply;

class CloudSyncService : public QObject {
    Q_OBJECT
public:
    explicit CloudSyncService(QObject* parent=nullptr);

    void setServerUrl(const QUrl& url);
    QUrl serverUrl() const;

    void testConnection();
    void uploadUserFiles(const QString& username, int userId);
    void downloadUserFiles(const QString& username, int userId);

signals:
    void statusMessage(const QString& message);
    void operationFinished(bool ok, const QString& message);

private:
    QUrl endpoint(const QString& path) const;
    QString encodedSegment(const QString& value) const;
    void finish(bool ok, const QString& message);
    void handleUploadReply(QNetworkReply* reply);
    void handleManifestReply(const QString& username, QNetworkReply* reply);
    void downloadNextFile();
    void handleDownloadReply(const QString& filename, QNetworkReply* reply);

    QNetworkAccessManager* manager_{};
    QUrl serverUrl_{QStringLiteral("http://127.0.0.1:18080")};
    int pendingUploads_{0};
    int uploadedFiles_{0};
    bool uploadHadError_{false};
    QString activeDownloadUser_;
    int activeDownloadUserId_{-1};
    QStringList downloadQueue_;
    int downloadedFiles_{0};
};
