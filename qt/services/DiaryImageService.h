#pragma once

#include <QString>
#include <QStringList>

class DiaryImageService {
public:
    static QString importImage(const QString& user, const QString& sourcePath);
    static QString imagePath(const QString& user, const QString& reference);
    static QString markerForImage(const QString& reference);
    static QStringList imageReferencesInText(const QString& text);
    static QStringList moveUnreferencedImagesToTrash(const QString& user);
    static bool isManagedImageFile(const QString& filename);
};
