#pragma once
#include <QWidget>
class QLabel; class QPushButton; class QTimer; class QLineEdit;

class StopwatchPage : public QWidget {
    Q_OBJECT
public:
    // 创建正向计时页面
    explicit StopwatchPage(QWidget* parent=nullptr);
private:
    QLabel* display_{}; QPushButton* btnStart_{}; QPushButton* btnStop_{}; QPushButton* btnReset_{}; QLineEdit* remark_{};
    QTimer* timer_{}; int elapsedSec_{0};
    // 初始化 UI
    void setupUi();
    // 开始计时
    void start();
    // 停止计时并记录
    void stop();
    // 重置计时
    void reset();
    // 每秒刷新计时显示
    void tick();
    // 将本次计时写入日记
    void record();
};
