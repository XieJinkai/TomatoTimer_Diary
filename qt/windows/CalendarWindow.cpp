#include "CalendarWindow.h"
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDate>
#include "DiaryWindow.h"
#include "../services/Session.h"

CalendarWindow::CalendarWindow(bool pickOnly, QWidget* parent) : QMainWindow(parent), pickOnly_(pickOnly){
    setupUi();
}

void CalendarWindow::setupUi(){
    auto* w = new QWidget(this);
    auto* lay = new QVBoxLayout(w);
    calendar_ = new QCalendarWidget(w);
    lay->addWidget(calendar_);
    w->setLayout(lay);
    setCentralWidget(w);
    setWindowTitle("日历");
    resize(480, 360);
    connect(calendar_, &QCalendarWidget::selectionChanged, this, &CalendarWindow::onDateSelected);
    connect(calendar_, &QCalendarWidget::clicked, this, [this](const QDate& d){
        if(!Session::instance().isLoggedIn()) return;
        if(pickOnly_){ emit datePicked(d); return; }
        DiaryWindow* dlg = new DiaryWindow(Session::instance().username(), d, this);
        dlg->setModal(true);
        dlg->show();
    });
}

void CalendarWindow::onDateSelected(){
    if(!Session::instance().isLoggedIn()) return;
    const auto d = calendar_->selectedDate();
    if(pickOnly_){ emit datePicked(d); return; }
    DiaryWindow* dlg = new DiaryWindow(Session::instance().username(), d, this);
    dlg->setModal(true);
    dlg->show();
}
