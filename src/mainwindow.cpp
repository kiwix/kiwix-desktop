
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QWebEngineProfile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mp_ui(new Ui::MainWindow),
    mp_about(new About(this))
{
    mp_ui->setupUi(this);
    mp_ui->tabWidget->tabBar()->setExpanding(false);
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::ExitAction), &QAction::triggered,
            this, &QMainWindow::close);
    connect(app->getAction(KiwixApp::ToggleFullscreenAction), &QAction::triggered,
            this, &MainWindow::toggleFullScreen);
#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
}

MainWindow::~MainWindow()
{
    delete mp_ui;
}

void MainWindow::toggleFullScreen() {
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

TabWidget* MainWindow::getTabWidget()
{
    return mp_ui->tabWidget;
}

