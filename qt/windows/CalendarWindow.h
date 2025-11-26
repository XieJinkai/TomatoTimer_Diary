#pragma once
#include <QMainWindow>
#include <QDate>
class QCalendarWidget;

class CalendarWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CalendarWindow(bool pickOnly=false, QWidget* parent=nullptr);
signals:
    void datePicked(const QDate&);
private:
    bool pickOnly_{};
    QCalendarWidget* calendar_{};
    void setupUi(); void onDateSelected();
};
