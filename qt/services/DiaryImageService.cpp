#include "DiaryImageService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSet>

#include "DataStore.h"

namespace {

QString normalizedReference(const QString& reference){
    return reference.trimmed();
}

QString extensionFor(const QString& path){
    const QString suffix = QFileInfo(path).suffix().toLower();
    if(suffix == "jpg" || suffix == "jpeg" || suffix == "png"){
        return suffix;
    }
    return {};
}

QString uniqueImageName(const QString& extension){
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    const quint32 token = QRandomGenerator::global()->generate();
    return QString("img_%1_%2.%3")
        .arg(stamp)
        .arg(QString::number(token, 16))
        .arg(extension);
}

QRegularExpression imageMarkerPattern(){
    return QRegularExpression(QStringLiteral("!\\[image\\]\\(([^\\)\\r\\n]+)\\)"));
}

QStringList referencedImagesInUserDir(const QString& userDir){
    QSet<QString> references;
    const QFileInfoList diaryFiles = QDir(userDir).entryInfoList(QStringList{"*.txt"}, QDir::Files, QDir::Name);
    for(const QFileInfo& info : diaryFiles){
        QFile file(info.filePath());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            continue;
        }
        const QString content = QString::fromUtf8(file.readAll());
        for(const QString& reference : DiaryImageService::imageReferencesInText(content)){
            references.insert(reference);
        }
        const auto lines = content.split('\n');
        for(const QString& line : lines){
            const QString trimmed = line.trimmed();
            if(trimmed.startsWith(QStringLiteral("[Image]"))){
                references.insert(normalizedReference(trimmed.mid(7)));
            }
        }
    }
    QStringList out = references.values();
    out.sort();
    return out;
}

}

QString DiaryImageService::importImage(const QString& user, const QString& sourcePath){
    const QString extension = extensionFor(sourcePath);
    if(user.trimmed().isEmpty() || sourcePath.trimmed().isEmpty() || extension.isEmpty()){
        return {};
    }

    const QString userDir = DataStore::userDir(user);
    const QFileInfo source(sourcePath);
    if(!source.exists() || !source.isFile()){
        return {};
    }
    if(source.absolutePath() == QFileInfo(userDir).absoluteFilePath() &&
       isManagedImageFile(source.fileName())){
        return source.fileName();
    }

    QString filename;
    QString targetPath;
    do {
        filename = uniqueImageName(extension);
        targetPath = userDir + QDir::separator() + filename;
    } while(QFile::exists(targetPath));

    if(!QFile::copy(sourcePath, targetPath)){
        return {};
    }
    return filename;
}

QString DiaryImageService::imagePath(const QString& user, const QString& reference){
    const QString normalized = normalizedReference(reference);
    if(normalized.isEmpty()){
        return {};
    }
    const QFileInfo info(normalized);
    if(info.isAbsolute()){
        return normalized;
    }
    return DataStore::userDir(user) + QDir::separator() + normalized;
}

QString DiaryImageService::markerForImage(const QString& reference){
    const QString normalized = QFileInfo(normalizedReference(reference)).fileName();
    return normalized.isEmpty() ? QString() : QString("![image](%1)").arg(normalized);
}

QStringList DiaryImageService::imageReferencesInText(const QString& text){
    QStringList references;
    const QRegularExpression re = imageMarkerPattern();
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()){
        const QString reference = normalizedReference(it.next().captured(1));
        if(!reference.isEmpty()){
            references.append(QFileInfo(reference).fileName());
        }
    }
    references.removeDuplicates();
    references.sort();
    return references;
}

QStringList DiaryImageService::moveUnreferencedImagesToTrash(const QString& user){
    const QString userDir = DataStore::userDir(user);
    QSet<QString> references;
    const QStringList referenced = referencedImagesInUserDir(userDir);
    for(const QString& reference : referenced){
        references.insert(reference);
    }

    QStringList moved;
    const QFileInfoList files = QDir(userDir).entryInfoList(QDir::Files, QDir::Name);
    for(const QFileInfo& fileInfo : files){
        const QString filename = fileInfo.fileName();
        if(!isManagedImageFile(filename) || references.contains(filename)){
            continue;
        }

        const QString trashDir = userDir + QDir::separator() + "image_trash";
        QDir().mkpath(trashDir);
        QString targetPath = trashDir + QDir::separator() + filename;
        if(QFile::exists(targetPath)){
            QFile::remove(targetPath);
        }
        if(QFile::rename(fileInfo.filePath(), targetPath)){
            moved.append(filename);
        }
    }
    moved.sort();
    return moved;
}

bool DiaryImageService::isManagedImageFile(const QString& filename){
    const QFileInfo info(filename);
    const QString suffix = info.suffix().toLower();
    return info.fileName().startsWith("img_") &&
           (suffix == "png" || suffix == "jpg" || suffix == "jpeg");
}
