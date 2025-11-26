#pragma once
#include <QDialog>
#include <QDate>
class QTextEdit; class QPushButton; class QLabel;
class QFontComboBox; class QComboBox;

class DiaryWindow : public QDialog {
    Q_OBJECT
public:
    explicit DiaryWindow(const QString& user, const QDate& date, QWidget* parent=nullptr);
private:
    QString user_{}; QDate date_{};
    QLabel* title_{}; QTextEdit* editor_{}; QPushButton* btnSave_{}; QPushButton* btnInsertImage_{}; QPushButton* btnVoice_{}; QPushButton* btnCalendar_{};
    QFontComboBox* fontBox_{}; QComboBox* sizeBox_{}; QPushButton* btnBold_{}; QPushButton* btnItalic_{};
    QStringList images_{};
    void setupUi(); void load(); void save(); void insertImage(); void voiceInput(); void openCalendar();
    void applyFontFamily(const QFont&); void applyFontSize(int); void toggleBold(); void toggleItalic();
};
