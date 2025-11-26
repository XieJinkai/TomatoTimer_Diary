#pragma once
#include <QMainWindow>
#include <QDate>
class QCalendarWidget;

class CalendarWindow : public QMainWindow {
    Q_OBJECT
public:
    // 创建日历窗口；pickOnly 为 true 时仅选择日期
    explicit CalendarWindow(bool pickOnly=false, QWidget* parent=nullptr);
signals:
    // 选择日期完成后发出
    void datePicked(const QDate&);
private:
    bool pickOnly_{};
    QCalendarWidget* calendar_{};
    // 初始化 UI
    void setupUi();
    // 处理日期选择事件
    void onDateSelected();
};
