#include "MainWindow.h"

#include <QAction>
#include <QIcon>
#include <QSizePolicy>
#include <QTabWidget>
#include <QToolBar>
#include <QWidget>

#include "LoginWindow.h"
#include "pages/AccountingPage.h"
#include "pages/DiaryTabPage.h"
#include "pages/ImageToolsPage.h"
#include "pages/PomodoroPage.h"
#include "pages/SettingsSyncPage.h"
#include "pages/StatsPage.h"
#include "pages/StopwatchPage.h"
#include "services/Session.h"
#include "ui/Theme.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setWindowTitle("TomatoTimer 桌面版");
    resize(1080, 720);
}

void MainWindow::setupUi(){
    tabs_ = new QTabWidget(this);
    tabs_->setObjectName("mainTabs");
    setCentralWidget(tabs_);

    tabs_->addTab(new PomodoroPage(this), QIcon(), "番茄钟");
    tabs_->addTab(new StopwatchPage(this), QIcon(), "正向计时");
    tabs_->addTab(new StatsPage(this), QIcon(), "数据统计");
    tabs_->addTab(new AccountingPage(this), QIcon(), "记账");

    auto* diaryTab = new DiaryTabPage(this);
    tabs_->addTab(diaryTab, QIcon(), "日记");
    tabs_->addTab(new ImageToolsPage(this), QIcon(), "图片处理");
    tabs_->addTab(new SettingsSyncPage(this), QIcon(), "设置/同步");

    auto* toolbar = addToolBar("工具");
    auto* actionLight = new QAction("日间主题", this);
    auto* actionDark = new QAction("夜间主题", this);
    auto* actionLogout = new QAction("退出登录", this);

    auto* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    toolbar->addSeparator();
    toolbar->addAction(actionLight);
    toolbar->addAction(actionDark);
    toolbar->addWidget(spacer);
    toolbar->addAction(actionLogout);

    connect(actionLight, &QAction::triggered, this, []{
        Theme::apply(Theme::Mode::Light);
        Theme::save(Theme::Mode::Light);
    });
    connect(actionDark, &QAction::triggered, this, []{
        Theme::apply(Theme::Mode::Dark);
        Theme::save(Theme::Mode::Dark);
    });
    connect(actionLogout, &QAction::triggered, this, [this]{
        Session::instance().logout();
        auto* loginWindow = new LoginWindow();
        loginWindow->show();
        close();
    });
}
