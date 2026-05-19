#include <QtTest/QtTest>

#include <QDir>
#include <QImage>
#include <QTextEdit>

#include "../services/DataStore.h"
#include "../services/DiaryDocumentCodec.h"

class DiaryDocumentCodecTests : public QObject {
    Q_OBJECT

private slots:
    void roundTripsImageMarkersThroughTextEditDocument(){
        const QString username = "diary_document_codec_test";
        const QString userDir = DataStore::userDir(username);
        QDir(userDir).removeRecursively();
        QDir().mkpath(userDir);

        QImage image(10, 10, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        QVERIFY(image.save(userDir + QDir::separator() + "img_keep.png"));

        QTextEdit editor;
        DiaryDocumentCodec::setDiaryText(&editor, username, "before\n![image](img_keep.png)\nafter");

        const QString saved = DiaryDocumentCodec::toDiaryText(editor.document());

        QCOMPARE(saved, QString("before\n![image](img_keep.png)\nafter"));
        QVERIFY(!saved.contains(userDir));

        QDir(userDir).removeRecursively();
    }
};

QTEST_MAIN(DiaryDocumentCodecTests)

#include "DiaryDocumentCodecTests.moc"
