
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QWebEngineProfile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mp_ui(new Ui::MainWindow)
{
    mp_ui->setupUi(this);
    mp_ui->tabWidget->tabBar()->setExpanding(false);
#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
}

MainWindow::~MainWindow()
{
    delete mp_ui;
}

TabWidget* MainWindow::getTabWidget()
{
    return mp_ui->tabWidget;
}

