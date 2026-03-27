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
    QComboBox { background: #FFFFFF; border: 1px solid #E5E5E5; border-radius: 8px; padding: 6px 34px 6px 10px; }
    QComboBox:hover { border-color: #CFCFCF; }
    QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 28px; border-left: 1px solid #E5E5E5; background: #F5F5F5; border-top-right-radius: 8px; border-bottom-right-radius: 8px; }
    QComboBox::down-arrow { image: url(:/qt-project.org/styles/commonstyle/images/arrowdown-16.png); width: 12px; height: 12px; }
    QComboBox QAbstractItemView { background: #FFFFFF; color: #1F1F1F; selection-background-color: #E6F4FF; selection-color: #1F1F1F; border: 1px solid #E5E5E5; outline: 0px; }
    QTableWidget { background: #FFFFFF; alternate-background-color: #F7F8FA; gridline-color: #E5E5E5; border: 1px solid #E5E5E5; border-radius: 8px; }
    QTableWidget::item { padding: 6px 10px; }
    QTableWidget::item:selected { background: #E6F4FF; color: #1F1F1F; }
    QHeaderView::section { background: #FAFAFA; color: #1F1F1F; padding: 6px 10px; border: none; border-bottom: 1px solid #E5E5E5; }
    QTableCornerButton::section { background: #FAFAFA; border: none; border-bottom: 1px solid #E5E5E5; }
    QMenu { background: #FFFFFF; color: #1F1F1F; border: 1px solid #E5E5E5; padding: 4px; }
    QMenu::item { padding: 8px 18px; border-radius: 6px; }
    QMenu::item:selected { background: #E6F4FF; color: #1F1F1F; }
    QMenu::separator { height: 1px; background: #E5E5E5; margin: 4px 8px; }
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
    QComboBox { background: #23262A; color: #EDEFF2; border: 1px solid #3A3E43; border-radius: 8px; padding: 6px 34px 6px 10px; }
    QComboBox:hover { border-color: #4A4F55; }
    QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 28px; border-left: 1px solid #3A3E43; background: #E0E2E6; border-top-right-radius: 8px; border-bottom-right-radius: 8px; }
    QComboBox::down-arrow { image: url(:/qt-project.org/styles/commonstyle/images/arrowdown-16.png); width: 12px; height: 12px; }
    QComboBox QAbstractItemView { background: #1F2226; color: #EDEFF2; selection-background-color: #2F343A; selection-color: #FFFFFF; border: 1px solid #3A3E43; outline: 0px; }
    QTableWidget { background: #1E2023; color: #EDEFF2; alternate-background-color: #202327; gridline-color: #343A40; border: 1px solid #2C2F33; border-radius: 8px; }
    QTableWidget::item { padding: 6px 10px; }
    QTableWidget::item:selected { background: #2F343A; color: #FFFFFF; }
    QHeaderView::section { background: #25282C; color: #F2F3F5; padding: 6px 10px; border: none; border-bottom: 1px solid #343A40; }
    QTableCornerButton::section { background: #25282C; border: none; border-bottom: 1px solid #343A40; }
    QMenu { background: #1F2226; color: #EDEFF2; border: 1px solid #2E3237; padding: 4px; }
    QMenu::item { padding: 8px 18px; border-radius: 6px; }
    QMenu::item:selected { background: #2F343A; color: #FFFFFF; }
    QMenu::separator { height: 1px; background: #343A40; margin: 4px 8px; }
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
