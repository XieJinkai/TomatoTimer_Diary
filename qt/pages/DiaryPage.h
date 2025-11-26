#pragma once
#include <QWidget>
class QCalendarWidget; class QTextEdit; class QPushButton; class QLineEdit;

class DiaryPage : public QWidget {
    Q_OBJECT
public:
    // 创建日记页面（带日历）
    explicit DiaryPage(QWidget* parent=nullptr);
private:
    QCalendarWidget* calendar_{}; QTextEdit* editor_{}; QLineEdit* mediaInfo_{}; QPushButton* btnSave_{}; QPushButton* btnInsertImage_{}; QPushButton* btnAttachAudio_{};
    // 初始化 UI
    void setupUi();
    // 加载所选日期的日记
    void loadSelected();
    // 保存当前日记与附件信息
    void save();
    // 插入图片为内嵌显示，路径记录到附件
    void insertImage();
    // 附加音频文件，路径记录到附件
    void attachAudio();
};
