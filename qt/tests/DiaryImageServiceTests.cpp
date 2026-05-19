#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTemporaryDir>

#include "../services/DataStore.h"
#include "../services/DiaryImageService.h"

class DiaryImageServiceTests : public QObject {
    Q_OBJECT

private slots:
    void importImageCopiesFileIntoUserDirectory(){
        const QString username = "diary_image_import_test";
        const QString userDir = DataStore::userDir(username);
        QDir(userDir).removeRecursively();
        QDir().mkpath(userDir);

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString sourcePath = tempDir.filePath("source.png");
        QImage image(12, 8, QImage::Format_ARGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(sourcePath));

        const QString filename = DiaryImageService::importImage(username, sourcePath);

        QVERIFY(filename.startsWith("img_"));
        QCOMPARE(QFileInfo(filename).isAbsolute(), false);
        QCOMPARE(QFileInfo(filename).suffix().toLower(), QString("png"));
        QVERIFY(QFile::exists(userDir + QDir::separator() + filename));
        QVERIFY(!filename.contains(tempDir.path()));

        QDir(userDir).removeRecursively();
    }

    void cleanupMovesOnlyUnreferencedManagedImagesToTrash(){
        const QString username = "diary_image_cleanup_test";
        const QString userDir = DataStore::userDir(username);
        QDir(userDir).removeRecursively();
        QDir().mkpath(userDir);

        QVERIFY(DataStore::writeAll(userDir + QDir::separator() + "2026-05-19.txt",
                                    "today\n![image](img_keep.png)\n"));
        QVERIFY(DataStore::writeAll(userDir + QDir::separator() + "img_keep.png", "keep"));
        QVERIFY(DataStore::writeAll(userDir + QDir::separator() + "img_remove.jpg", "remove"));
        QVERIFY(DataStore::writeAll(userDir + QDir::separator() + "notes.txt", "not managed image"));

        const QStringList moved = DiaryImageService::moveUnreferencedImagesToTrash(username);

        QCOMPARE(moved, QStringList{"img_remove.jpg"});
        QVERIFY(QFile::exists(userDir + QDir::separator() + "img_keep.png"));
        QVERIFY(!QFile::exists(userDir + QDir::separator() + "img_remove.jpg"));
        QVERIFY(QFile::exists(userDir + QDir::separator() + "image_trash" + QDir::separator() + "img_remove.jpg"));
        QVERIFY(QFile::exists(userDir + QDir::separator() + "notes.txt"));

        QDir(userDir).removeRecursively();
    }
};

QTEST_MAIN(DiaryImageServiceTests)

#include "DiaryImageServiceTests.moc"
