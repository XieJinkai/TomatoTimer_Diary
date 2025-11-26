#pragma once
#include <QWidget>
class QCalendarWidget; class QTextEdit; class QPushButton; class QLineEdit;

class DiaryPage : public QWidget {
    Q_OBJECT
public:
    explicit DiaryPage(QWidget* parent=nullptr);
private:
    QCalendarWidget* calendar_{}; QTextEdit* editor_{}; QLineEdit* mediaInfo_{}; QPushButton* btnSave_{}; QPushButton* btnInsertImage_{}; QPushButton* btnAttachAudio_{};
    void setupUi(); void loadSelected(); void save(); void insertImage(); void attachAudio();
};