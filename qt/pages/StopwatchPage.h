#pragma once
#include <QWidget>
class QLabel; class QPushButton; class QTimer; class QLineEdit;

class StopwatchPage : public QWidget {
    Q_OBJECT
public:
    explicit StopwatchPage(QWidget* parent=nullptr);
private:
    QLabel* display_{}; QPushButton* btnStart_{}; QPushButton* btnStop_{}; QPushButton* btnReset_{}; QLineEdit* remark_{};
    QTimer* timer_{}; int elapsedSec_{0};
    void setupUi(); void start(); void stop(); void reset(); void tick(); void record();
};