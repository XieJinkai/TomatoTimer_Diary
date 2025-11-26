#pragma once
#include <QWidget>
class QLineEdit; class QSpinBox; class QPushButton; class QLabel; class QTimer;

class PomodoroPage : public QWidget {
    Q_OBJECT
public:
    // 创建番茄钟页面
    explicit PomodoroPage(QWidget* parent=nullptr);
private:
    QLineEdit* taskName_{}; QSpinBox* count_{}; QSpinBox* focusMin_{}; QSpinBox* restMin_{};
    QPushButton* btnStart_{}; QPushButton* btnPause_{}; QPushButton* btnResume_{}; QPushButton* btnEndEarly_{}; QLabel* display_{};
    QTimer* timer_{}; int remainingSec_{0}; int currentRound_{0}; bool inRest_{false};
    // 初始化 UI
    void setupUi();
    // 开始一轮番茄
    void start();
    // 暂停计时
    void pause();
    // 继续计时
    void resume();
    // 每秒刷新计时显示
    void tick();
    // 记录一次完成的专注分钟
    void recordDone(int minutes);
    // 提前结束当前阶段（记录当前专注时长并进入下一阶段）
    void endEarly();
};
