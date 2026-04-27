#pragma once

#include <QDateTime>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTimer;

class PomodoroPage : public QWidget {
    Q_OBJECT
public:
    explicit PomodoroPage(QWidget* parent=nullptr);

private:
    QLineEdit* taskName_{};
    QSpinBox* count_{};
    QSpinBox* focusMin_{};
    QSpinBox* restMin_{};
    QPushButton* btnStart_{};
    QPushButton* btnPause_{};
    QPushButton* btnResume_{};
    QPushButton* btnEndCurrent_{};
    QPushButton* btnSkipRest_{};
    QPushButton* btnEndPlan_{};
    QPushButton* btnReset_{};
    QLabel* phaseLabel_{};
    QLabel* display_{};
    QTimer* timer_{};
    int remainingSec_{0};
    int currentRound_{0};
    bool inRest_{false};
    QDateTime focusStartedAt_{};

    void setupUi();
    void start();
    void pause();
    void resume();
    void tick();
    void endCurrentFocus();
    void skipRest();
    void endPlan();
    void reset();
    void beginFocusRound(int round);
    void beginRest();
    void finishPlan();
    void updateDisplay();
    void updateControls();
    void recordFocus(const QString& status, int actualSec);
};
