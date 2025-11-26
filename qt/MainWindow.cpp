#include "MainWindow.h"
#include <QTabWidget>
#include <QWidget>
#include <QIcon>
#include "pages/LoginRegisterPage.h"
#include "pages/PomodoroPage.h"
#include "pages/StopwatchPage.h"
#include <QToolBar>
#include <QAction>
#include "windows/CalendarWindow.h"
#include "ui/Theme.h"
#include "pages/DiaryTabPage.h"
#include <QMessageBox>
#include "windows/DiaryWindow.h"
#include <QDate>
#include "services/Session.h"
#include "pages/StatsPage.h"
#include "pages/ImageToolsPage.h"
#include "pages/SettingsSyncPage.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setWindowTitle("TomatoTimer 桌面版");
    resize(1080, 720);
}

void MainWindow::setupUi(){
    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    tabs_->addTab(new PomodoroPage(this), QIcon(), "番茄钟");
    tabs_->addTab(new StopwatchPage(this), QIcon(), "正向计时");
    tabs_->addTab(new StatsPage(this), QIcon(), "数据统计");
    auto* diaryTab = new DiaryTabPage(this);
    tabs_->addTab(diaryTab, QIcon(), "日记");
    tabs_->addTab(new ImageToolsPage(this), QIcon(), "图片处理");
    tabs_->addTab(new SettingsSyncPage(this), QIcon(), "设置/同步");

    auto* tb = addToolBar("工具");
    QAction* actLight = new QAction("日间主题", this);
    QAction* actDark = new QAction("夜间主题", this);
    tb->addSeparator();
    tb->addAction(actLight);
    tb->addAction(actDark);
    connect(actLight, &QAction::triggered, this, []{
        Theme::apply(Theme::Mode::Light);
        Theme::save(Theme::Mode::Light);
    });
    connect(actDark, &QAction::triggered, this, []{
        Theme::apply(Theme::Mode::Dark);
        Theme::save(Theme::Mode::Dark);
    });
}
