#pragma once
#include <QWidget>
class QLabel;
class StatsPage : public QWidget {
    Q_OBJECT
public:
    explicit StatsPage(QWidget* parent=nullptr);
private:
    QLabel* lbl_{};
    void setupUi(); void refresh();
};