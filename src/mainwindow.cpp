
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_about.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QDesktopServices>
#ifdef Q_OS_WIN
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mp_ui(new Ui::MainWindow),
    mp_about(new About(this)),
    mp_localKiwixServer(new LocalKiwixServer(this))
{
    mp_ui->setupUi(this);
    mp_ui->tabBar->setExpanding(false);
    mp_ui->tabBar->setStackedWidget(mp_ui->mainView);
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::ExitAction), &QAction::triggered,
            this, &QMainWindow::close);
    connect(app->getAction(KiwixApp::ToggleFullscreenAction), &QAction::triggered,
            this, &MainWindow::toggleFullScreen);
    connect(app->getAction(KiwixApp::AboutAction), &QAction::triggered,
            mp_about, &QDialog::show);
    connect(app->getAction(KiwixApp::DonateAction), &QAction::triggered,
            this, [=]() { QDesktopServices::openUrl(QUrl("https://donate.kiwix.org")); });
    connect(app->getAction(KiwixApp::KiwixServeAction), &QAction::triggered,
            mp_localKiwixServer, &QDialog::show);
    connect(app, &KiwixApp::currentTitleChanged, this, [=](const QString& title) {
        if (!title.isEmpty() && !title.startsWith("zim://"))
            setWindowTitle(title + " - Kiwix");
        else
            setWindowTitle(gt("window-title"));
    });
    addAction(app->getAction(KiwixApp::OpenHomePageAction));
#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
#ifdef Q_OS_WIN
    QWindow *window = windowHandle();
    if (!window) {
        return;
    }
    QWindowsWindowFunctions::setHasBorderInFullScreen(window, true);
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    auto key = event->key();
    auto modifier = event->modifiers();
    if (key == Qt::Key_F6 ||
        (key == Qt::Key_L && modifier == Qt::ControlModifier) ||
        (key == Qt::Key_D && modifier == Qt::AltModifier)) {
        getTopWidget()->getSearchBar().selectAll();
        getTopWidget()->getSearchBar().setFocus();
    }
    return QWidget::keyPressEvent(event);
}

TabBar* MainWindow::getTabBar()
{
    return mp_ui->tabBar;
}

TopWidget *MainWindow::getTopWidget()
{
    return mp_ui->mainToolBar;
}

QStackedWidget *MainWindow::getSideDockWidget()
{
    return mp_ui->sideBar;
}

ContentManagerSide *MainWindow::getSideContentManager()
{
    return mp_ui->contentmanagerside;
}
