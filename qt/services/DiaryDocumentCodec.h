#pragma once

#include <QString>

class QTextDocument;
class QTextEdit;

class DiaryDocumentCodec {
public:
    static void setDiaryText(QTextEdit* editor, const QString& user, const QString& text);
    static QString toDiaryText(const QTextDocument* document);
    static void insertImage(QTextEdit* editor, const QString& user, const QString& filename);
};
