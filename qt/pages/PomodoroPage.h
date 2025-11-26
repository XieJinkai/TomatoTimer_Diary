#pragma once
#include <QWidget>
class QLineEdit; class QSpinBox; class QPushButton; class QLabel; class QTimer;

class PomodoroPage : public QWidget {
    Q_OBJECT
public:
    explicit PomodoroPage(QWidget* parent=nullptr);
private:
    QLineEdit* taskName_{}; QSpinBox* count_{}; QSpinBox* focusMin_{}; QSpinBox* restMin_{};
    QPushButton* btnStart_{}; QPushButton* btnPause_{}; QPushButton* btnResume_{}; QLabel* display_{};
    QTimer* timer_{}; int remainingSec_{0}; int currentRound_{0}; bool inRest_{false};
    void setupUi();
    void start(); void pause(); void resume(); void tick();
    void recordDone(int minutes);
};