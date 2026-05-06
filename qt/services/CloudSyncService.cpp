#include "CloudSyncService.h"

#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "DataStore.h"

CloudSyncService::CloudSyncService(QObject* parent)
    : QObject(parent), manager_(new QNetworkAccessManager(this)) {}

void CloudSyncService::setServerUrl(const QUrl& url){
    if(url.isValid() && !url.isEmpty()){
        serverUrl_ = url;
    }
}

QUrl CloudSyncService::serverUrl() const {
    return serverUrl_;
}

void CloudSyncService::testConnection(){
    emit statusMessage("正在测试服务器连接...");
    QNetworkReply* reply = manager_->get(QNetworkRequest(endpoint("/health")));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        const bool ok = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
        finish(ok, ok ? "服务器连接正常" : "服务器连接失败");
    });
}

void CloudSyncService::uploadUserFiles(const QString& username, int userId){
    if(userId <= 0){
        finish(false, "请使用注册账号登录后再同步到服务器");
        return;
    }

    const QString userDir = DataStore::userDir(username);
    const QFileInfoList files = QDir(userDir).entryInfoList(QDir::Files, QDir::Name);
    if(files.isEmpty()){
        finish(true, "没有需要上传的文件");
        return;
    }

    pendingUploads_ = files.size();
    uploadedFiles_ = 0;
    uploadHadError_ = false;
    emit statusMessage(QString("正在上传 %1 个文件...").arg(files.size()));

    for(const QFileInfo& fileInfo : files){
        QFile file(fileInfo.filePath());
        if(!file.open(QIODevice::ReadOnly)){
            uploadHadError_ = true;
            --pendingUploads_;
            continue;
        }

        QJsonObject payload;
        payload.insert("filename", fileInfo.fileName());
        payload.insert("contentBase64", QString::fromLatin1(file.readAll().toBase64()));

        QNetworkRequest request(endpoint(QString("/api/users/%1/files/upload").arg(userId)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = manager_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply](){
            handleUploadReply(reply);
        });
    }

    if(pendingUploads_ == 0){
        finish(false, "文件读取失败，未能上传");
    }
}

void CloudSyncService::downloadUserFiles(const QString& username, int userId){
    if(userId <= 0){
        finish(false, "请使用注册账号登录后再从服务器下载");
        return;
    }

    activeDownloadUser_ = username;
    activeDownloadUserId_ = userId;
    downloadQueue_.clear();
    downloadedFiles_ = 0;
    emit statusMessage("正在获取云端文件列表...");

    QNetworkReply* reply = manager_->get(
        QNetworkRequest(endpoint(QString("/api/users/%1/files").arg(userId)))
    );
    connect(reply, &QNetworkReply::finished, this, [this, username, reply](){
        handleManifestReply(username, reply);
    });
}

QUrl CloudSyncService::endpoint(const QString& path) const {
    QUrl url = serverUrl_;
    QString basePath = url.path();
    if(basePath.endsWith('/')){
        basePath.chop(1);
    }
    url.setPath(basePath + path);
    return url;
}

QString CloudSyncService::encodedSegment(const QString& value) const {
    return QString::fromLatin1(QUrl::toPercentEncoding(value));
}

void CloudSyncService::finish(bool ok, const QString& message){
    emit statusMessage(message);
    emit operationFinished(ok, message);
}

void CloudSyncService::handleUploadReply(QNetworkReply* reply){
    const bool ok = reply->error() == QNetworkReply::NoError;
    if(ok){
        ++uploadedFiles_;
    }else{
        uploadHadError_ = true;
    }
    reply->deleteLater();

    --pendingUploads_;
    if(pendingUploads_ == 0){
        if(uploadHadError_){
            finish(false, QString("上传完成，但有文件失败；成功 %1 个").arg(uploadedFiles_));
        }else{
            finish(true, QString("已上传 %1 个文件到云端").arg(uploadedFiles_));
        }
    }
}

void CloudSyncService::handleManifestReply(const QString& username, QNetworkReply* reply){
    if(reply->error() != QNetworkReply::NoError){
        reply->deleteLater();
        finish(false, "获取云端文件列表失败");
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    if(!doc.isArray()){
        finish(false, "云端文件列表格式错误");
        return;
    }

    for(const QJsonValue& value : doc.array()){
        const QString filename = value.toObject().value("filename").toString();
        if(!filename.isEmpty()){
            downloadQueue_.append(filename);
        }
    }

    if(downloadQueue_.isEmpty()){
        finish(true, "云端没有可下载的文件");
        return;
    }

    activeDownloadUser_ = username;
    emit statusMessage(QString("正在下载 %1 个文件...").arg(downloadQueue_.size()));
    downloadNextFile();
}

void CloudSyncService::downloadNextFile(){
    if(downloadQueue_.isEmpty()){
        finish(true, QString("已从云端下载 %1 个文件").arg(downloadedFiles_));
        return;
    }

    const QString filename = downloadQueue_.takeFirst();
    QNetworkReply* reply = manager_->get(
        QNetworkRequest(endpoint(QString("/api/users/%1/files/%2")
            .arg(activeDownloadUserId_).arg(encodedSegment(filename))))
    );
    connect(reply, &QNetworkReply::finished, this, [this, filename, reply](){
        handleDownloadReply(filename, reply);
    });
}

void CloudSyncService::handleDownloadReply(const QString& filename, QNetworkReply* reply){
    if(reply->error() != QNetworkReply::NoError){
        reply->deleteLater();
        finish(false, QString("下载文件失败：%1").arg(filename));
        return;
    }

    const QString userDir = DataStore::userDir(activeDownloadUser_);
    QFile file(userDir + QDir::separator() + filename);
    if(!file.open(QIODevice::WriteOnly)){
        reply->deleteLater();
        finish(false, QString("写入本地文件失败：%1").arg(filename));
        return;
    }

    file.write(reply->readAll());
    reply->deleteLater();
    ++downloadedFiles_;
    downloadNextFile();
}
