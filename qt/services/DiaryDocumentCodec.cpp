#include "DiaryDocumentCodec.h"

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QUrl>

#include "DiaryImageService.h"

namespace {

QRegularExpression imageMarkerPattern(){
    return QRegularExpression(QStringLiteral("!\\[image\\]\\(([^\\)\\r\\n]+)\\)"));
}

void insertImageReference(QTextCursor* cursor, QTextDocument* document, const QString& user, const QString& reference){
    const QString filename = QFileInfo(reference.trimmed()).fileName();
    const QString path = DiaryImageService::imagePath(user, filename);
    QImage image(path);
    if(image.isNull()){
        cursor->insertText(DiaryImageService::markerForImage(filename));
        return;
    }

    const QUrl resourceUrl(filename);
    document->addResource(QTextDocument::ImageResource, resourceUrl, image);

    QTextImageFormat format;
    format.setName(filename);
    if(image.width() > 680){
        format.setWidth(680);
    }
    cursor->insertImage(format);
}

}

void DiaryDocumentCodec::setDiaryText(QTextEdit* editor, const QString& user, const QString& text){
    if(editor == nullptr){
        return;
    }

    editor->clear();
    QTextCursor cursor(editor->document());
    const QStringList lines = text.split('\n');
    const QRegularExpression re = imageMarkerPattern();

    for(int i = 0; i < lines.size(); ++i){
        const QString& line = lines.at(i);
        int pos = 0;
        QRegularExpressionMatchIterator it = re.globalMatch(line);
        while(it.hasNext()){
            const QRegularExpressionMatch match = it.next();
            cursor.insertText(line.mid(pos, match.capturedStart() - pos));
            insertImageReference(&cursor, editor->document(), user, match.captured(1));
            pos = match.capturedEnd();
        }
        cursor.insertText(line.mid(pos));
        if(i + 1 < lines.size()){
            cursor.insertBlock();
        }
    }
}

QString DiaryDocumentCodec::toDiaryText(const QTextDocument* document){
    if(document == nullptr){
        return {};
    }

    QStringList lines;
    for(QTextBlock block = document->begin(); block.isValid(); block = block.next()){
        QString line;
        for(QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it){
            const QTextFragment fragment = it.fragment();
            if(!fragment.isValid()){
                continue;
            }
            const QTextCharFormat format = fragment.charFormat();
            if(format.isImageFormat()){
                line += DiaryImageService::markerForImage(format.toImageFormat().name());
            }else{
                line += fragment.text();
            }
        }
        lines.append(line);
    }
    return lines.join('\n');
}

void DiaryDocumentCodec::insertImage(QTextEdit* editor, const QString& user, const QString& filename){
    if(editor == nullptr || filename.trimmed().isEmpty()){
        return;
    }
    QTextCursor cursor = editor->textCursor();
    insertImageReference(&cursor, editor->document(), user, filename);
    editor->setTextCursor(cursor);
}
