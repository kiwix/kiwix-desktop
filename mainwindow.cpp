
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QWebEngineProfile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->tabBar()->setExpanding(false);
#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

KTabWidget* MainWindow::getTabWidget()
{
    return ui->tabWidget;
}

