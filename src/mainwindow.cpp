
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_about.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QDesktopServices>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mp_ui(new Ui::MainWindow),
    mp_about(new About(this)),
    mp_localKiwixServer(new LocalKiwixServer(this))
{
    QWidget::setAttribute(Qt::WA_AlwaysShowToolTips);
    mp_ui->setupUi(this);

    mp_ui->tabBar->setExpanding(false);
    mp_ui->tabBar->setStackedWidget(mp_ui->mainView);

    auto app = KiwixApp::instance();
    addAction(app->getAction(KiwixApp::ToggleFullscreenAction));
    addAction(app->getAction(KiwixApp::NextTabAction));
    addAction(app->getAction(KiwixApp::PreviousTabAction));

    connect(app->getAction(KiwixApp::ExitAction), &QAction::triggered,
            this, &QMainWindow::close);
    connect(app->getAction(KiwixApp::ToggleFullscreenAction), &QAction::triggered,
            this, &MainWindow::toggleFullScreen);
    connect(app->getAction(KiwixApp::ToggleReadingListAction), &QAction::toggled,
            this, &MainWindow::when_ReadingList_toggled);
    connect(app->getAction(KiwixApp::AboutAction), &QAction::triggered,
            mp_about, &QDialog::show);
    connect(app->getAction(KiwixApp::DonateAction), &QAction::triggered,
            this, [=]() { QDesktopServices::openUrl(QUrl("https://donate.kiwix.org")); });
    connect(app->getAction(KiwixApp::KiwixServeAction), &QAction::triggered,
            mp_localKiwixServer, &QDialog::show);

    connect(mp_ui->tabBar, &TabBar::currentTitleChanged, this, [=](const QString& title) {
        if (!title.isEmpty() && !title.startsWith("zim://"))
            setWindowTitle(title + " - Kiwix");
        else
            setWindowTitle(gt("window-title"));
    });

    addAction(app->getAction(KiwixApp::OpenHomePageAction));

#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif

    connect(mp_ui->tabBar, &QTabBar::currentChanged,
            mp_ui->mainToolBar, &TopWidget::updateBackForwardButtons);
    connect(mp_ui->tabBar, &TabBar::tabDisplayed,
            this, [=](TabType tabType) {
                when_libraryPageDisplayed(tabType == TabType::LibraryTab);
            });

    connect(mp_ui->tabBar, &TabBar::currentTitleChanged,
            &(mp_ui->mainToolBar->getSearchBar()), &SearchBar::on_currentTitleChanged);

    // This signal emited more often than the history really updated
    // but for now we have no better signal for it.
    connect(mp_ui->tabBar, &TabBar::currentTitleChanged,
            mp_ui->mainToolBar, &TopWidget::updateBackForwardButtons);

    connect(mp_ui->tabBar, &TabBar::webActionEnabledChanged,
            mp_ui->mainToolBar, &TopWidget::handleWebActionEnabledChanged);

    mp_ui->contentmanagerside->setContentManager(app->getContentManager());
    mp_ui->sideBar->setCurrentWidget(mp_ui->contentmanagerside);
}

MainWindow::~MainWindow()
{
    delete mp_ui;
}

void MainWindow::toggleFullScreen() {
    if (isFullScreen()) {
        QApplication::instance()->removeEventFilter(this);
        showTabAndTop();
        showNormal();
    }
    else {
        QApplication::instance()->installEventFilter(this);
        hideTabAndTop();
        showFullScreen();
    }
}

void MainWindow::hideTabAndTop() {
    getTabBar()->hide();
    getTopWidget()->hide();
}

void MainWindow::showTabAndTop() {
    getTabBar()->show();
    getTopWidget()->show();
}

bool MainWindow::eventFilter(QObject* /*object*/, QEvent* event)
{
    if (event->type() == QEvent::MouseMove && isFullScreen())
    {
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        const int tabRegion = getTabBar()->height() + getTopWidget()->height() + 30;
        // We don't have to check for visibilty as calling hide() on a hidden widget, or show() on a non-hidden widget is a no-op
        if (mouseEvent->y() == 0) {
            showTabAndTop();
        } else if(mouseEvent->y() >= tabRegion) {
            hideTabAndTop();
        }
        return true;
    }
    return false;
}


void MainWindow::when_ReadingList_toggled(bool state)
{
    if (state) {
        mp_ui->sideBar->setCurrentWidget(mp_ui->readinglistbar);
        mp_ui->sideBar->show();
    }
    else {
        mp_ui->sideBar->hide();
    }
}

void MainWindow::when_libraryPageDisplayed(bool showed)
{
    auto app = KiwixApp::instance();

    // When library sidebar appeared, or hidden, reading list is always hidden.
    app->getAction(KiwixApp::ToggleReadingListAction)->setChecked(false);

    if (showed) {
        mp_ui->sideBar->setCurrentWidget(mp_ui->contentmanagerside);
        mp_ui->sideBar->show();
    }
    else {
        mp_ui->sideBar->hide();
    }
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
