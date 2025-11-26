#include "Theme.h"
#include <QApplication>
#include <QSettings>

static QString lightCss(){
    return QString(R"(
    QMainWindow, QDialog, QWidget { background: #FAFAFA; color: #1F1F1F; font-family: Microsoft YaHei, Segoe UI, Arial; font-size: 14px; }
    QLabel#title { font-size: 18px; font-weight: 600; }
    QToolBar { background: #FFFFFF; border: none; spacing: 8px; padding: 8px; }
    QTabWidget::pane { border: 1px solid #E5E5E5; border-radius: 8px; }
    QTabBar::tab { background: #FFFFFF; padding: 8px 16px; margin: 2px; border: 1px solid #E5E5E5; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; }
    QTabBar::tab:selected { color: #FF4D4F; font-weight: 600; }
    QPushButton { background: #FFFFFF; border: 1px solid #E5E5E5; border-radius: 8px; padding: 8px 14px; }
    QPushButton:hover { border-color: #CFCFCF; }
    QPushButton:pressed { background: #F0F0F0; }
    QLineEdit, QTextEdit { background: #FFFFFF; border: 1px solid #E5E5E5; border-radius: 8px; padding: 6px 10px; }
    QCalendarWidget QWidget { background: #FFFFFF; }
    QCalendarWidget QAbstractItemView:enabled { selection-background-color: #FF4D4F; selection-color: #FFFFFF; }
    )");
}

static QString darkCss(){
    return QString(R"(
    QMainWindow, QDialog, QWidget { background: #1B1D1F; color: #E6E7E9; font-family: Microsoft YaHei, Segoe UI, Arial; font-size: 14px; }
    QLabel#title { font-size: 18px; font-weight: 600; color: #F2F3F5; }
    QToolBar { background: #202326; border: none; spacing: 8px; padding: 8px; }
    QTabWidget::pane { border: 1px solid #2C2F33; border-radius: 8px; }
    QTabBar::tab { background: #23262A; color: #CDD0D4; padding: 8px 16px; margin: 2px; border: 1px solid #2C2F33; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; }
    QTabBar::tab:selected { color: #E57D7A; font-weight: 600; background: #2A2D31; }
    QPushButton { background: #202326; color: #E6E7E9; border: 1px solid #2C2F33; border-radius: 8px; padding: 8px 14px; }
    QPushButton:hover { border-color: #3A3E43; }
    QPushButton:pressed { background: #2A2D31; }
    QLineEdit, QTextEdit { background: #23262A; color: #E6E7E9; border: 1px solid #2C2F33; border-radius: 8px; padding: 6px 10px; }
    QCalendarWidget QWidget { background: #23262A; color: #E6E7E9; }
    QCalendarWidget QAbstractItemView:enabled { selection-background-color: #E57D7A; selection-color: #FFFFFF; }
    )");
}

void Theme::apply(Mode m){
    if(m == Mode::Light) qApp->setStyleSheet(lightCss()); else qApp->setStyleSheet(darkCss());
}

void Theme::save(Mode m){
    QSettings s("Trae", "TomatoTimerQt");
    s.setValue("ui/theme", m == Mode::Light ? "light" : "dark");
}

Theme::Mode Theme::load(){
    QSettings s("Trae", "TomatoTimerQt");
    const QString v = s.value("ui/theme", "light").toString();
    return v == "dark" ? Mode::Dark : Mode::Light;
}
