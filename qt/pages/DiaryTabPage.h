#pragma once
#include <QWidget>
#include <QDate>
class QLabel; class QTextEdit; class QPushButton; class QFontComboBox; class QComboBox;

class DiaryTabPage : public QWidget {
    Q_OBJECT
public:
    // 创建日记 Tab 页
    explicit DiaryTabPage(QWidget* parent=nullptr);
private:
    QString user_{}; QDate date_{};
    QLabel* title_{}; QTextEdit* editor_{}; QPushButton* btnSave_{}; QPushButton* btnInsertImage_{}; QPushButton* btnVoice_{}; QPushButton* btnCalendar_{};
    QFontComboBox* fontBox_{}; QComboBox* sizeBox_{}; QPushButton* btnBold_{}; QPushButton* btnItalic_{};
    QStringList images_{};
    // 初始化 UI
    void setupUi();
    // 按日期加载内容
    void load();
    // 保存内容到存储
    void save();
    // 插入图片到编辑区与存储
    void insertImage();
    // 语音输入并追加到编辑区
    void voiceInput();
    // 打开日历窗口选择日期
    void openCalendar();
    // 字体设置与样式切换
    void applyFontFamily(const QFont&);
    void applyFontSize(int);
    void toggleBold();
    void toggleItalic();
};
