#pragma once
#include <QDialog>
#include <QDate>
class QTextEdit; class QPushButton; class QLabel;
class QFontComboBox; class QComboBox;

class DiaryWindow : public QDialog {
    Q_OBJECT
public:
    // 创建独立的日记窗口
    explicit DiaryWindow(const QString& user, const QDate& date, QWidget* parent=nullptr);
private:
    QString user_{}; QDate date_{};
    QLabel* title_{}; QTextEdit* editor_{}; QPushButton* btnSave_{}; QPushButton* btnInsertImage_{}; QPushButton* btnVoice_{}; QPushButton* btnCalendar_{};
    QFontComboBox* fontBox_{}; QComboBox* sizeBox_{}; QPushButton* btnBold_{}; QPushButton* btnItalic_{};
    QStringList images_{};
    // 初始化 UI
    void setupUi();
    // 加载并展示内容
    void load();
    // 保存修改
    void save();
    // 插入图片
    void insertImage();
    // 语音输入
    void voiceInput();
    // 打开日历
    void openCalendar();
    // 样式设置
    void applyFontFamily(const QFont&);
    void applyFontSize(int);
    void toggleBold();
    void toggleItalic();
};
