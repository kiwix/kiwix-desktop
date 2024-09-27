#include "mainwindow.h"
#include "portutils.h"
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

    mp_ui->newTabSideButton->setDefaultAction(app->getAction(KiwixApp::NewTabAction));
    mp_ui->nextTabButton->setDefaultAction(app->getAction(KiwixApp::ScrollNextTabAction));
    mp_ui->prevTabButton->setDefaultAction(app->getAction(KiwixApp::ScrollPreviousTabAction));

    connect(mp_ui->tabBar, &TabBar::sizeChanged, this, &MainWindow::updateTabButtons);
    connect(mp_ui->tabBar, &QTabBar::currentChanged, this, &MainWindow::updateTabButtons);
    connect(mp_ui->tabBar, &TabBar::tabRemovedSignal, this, &MainWindow::updateTabButtons);
    connect(mp_ui->tabBar, &TabBar::tabInsertedSignal, this, &MainWindow::updateTabButtons);

    connect(mp_ui->nextTabButton, &QToolButton::triggered, mp_ui->tabBar, &TabBar::scrollNextTab);
    connect(mp_ui->prevTabButton, &QToolButton::triggered, mp_ui->tabBar, &TabBar::scrollPreviousTab);

    connect(app->getAction(KiwixApp::ExitAction), &QAction::triggered,
            this, &QMainWindow::close);
    connect(app->getAction(KiwixApp::ToggleFullscreenAction), &QAction::triggered,
            this, &MainWindow::toggleFullScreen);
    connect(app->getAction(KiwixApp::ToggleReadingListAction), &QAction::toggled,
            this, &MainWindow::readingListToggled);
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
                tabChanged(tabType);
            });

    connect(mp_ui->tabBar, &TabBar::currentTitleChanged,
            &(mp_ui->mainToolBar->getSearchBar()), &SearchBar::currentTitleChanged);
    // This signal emited more often than the history really updated
    // but for now we have no better signal for it.
    connect(mp_ui->tabBar, &TabBar::currentTitleChanged,
            mp_ui->mainToolBar, &TopWidget::updateBackForwardButtons);

    connect(mp_ui->tabBar, &TabBar::webActionEnabledChanged,
            mp_ui->mainToolBar, &TopWidget::handleWebActionEnabledChanged);

    mp_ui->contentmanagerside->setContentManager(app->getContentManager());
    mp_ui->sideBar->setCurrentWidget(mp_ui->contentmanagerside);

    // Add a new action in the UI to toggle the private mode status
    QAction *togglePrivateModeAction = new QAction(tr("Toggle Private Mode"), this);
    connect(togglePrivateModeAction, &QAction::triggered, this, &MainWindow::togglePrivateMode);
    mp_ui->mainToolBar->addAction(togglePrivateModeAction);
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

void MainWindow::updateTabButtons()
{
    auto tabBar = getTabBar();
    QRect tabBarRect = getTabBar()->rect();
    QRect newTabButtonRect = tabBar->tabRect(tabBar->count() - 1);

    /* Decision is made at half way of the new button tab for smoothness */
    newTabButtonRect.setWidth(newTabButtonRect.width() / 2);
    bool newTabButtonVisible = tabBarRect.contains(newTabButtonRect);
    mp_ui->newTabSideButton->setHidden(newTabButtonVisible);

    int lastTabIndex = tabBar->realTabCount() - 1;
    QRect firstTabRect = tabBar->tabRect(0);
    QRect lastTabRect = tabBar->tabRect(lastTabIndex);

    bool firstVisible = tabBarRect.contains(firstTabRect);
    bool lastVisible = tabBarRect.contains(lastTabRect);
    bool noOverFlow = firstVisible && lastVisible;

    mp_ui->prevTabButton->setHidden(noOverFlow);
    mp_ui->nextTabButton->setHidden(noOverFlow);

    mp_ui->prevTabButton->setDisabled(tabBar->currentIndex() == 0);
    mp_ui->nextTabButton->setDisabled(tabBar->currentIndex() == lastTabIndex);
}

bool MainWindow::eventFilter(QObject* /*object*/, QEvent* event)
{
    if (event->type() == QEvent::MouseMove && isFullScreen())
    {
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        const int tabRegion = getTabBar()->height() + getTopWidget()->height() + 30;
        int clickY = portutils::getY(*mouseEvent);
        // We don't have to check for visibilty as calling hide() on a hidden widget, or show() on a non-hidden widget is a no-op
        if (clickY == 0) {
            showTabAndTop();
        } else if(clickY >= tabRegion) {
            hideTabAndTop();
        }
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    KiwixApp::instance()->saveWindowState();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    KiwixApp::instance()->getContentManager()->getView()->updateSizeHint();
    updateTabButtons();
}

void MainWindow::readingListToggled(bool state)
{
    if (state) {
        mp_ui->sideBar->setCurrentWidget(mp_ui->readinglistbar);
        mp_ui->sideBar->show();
    }
    else {
        mp_ui->sideBar->hide();
    }
}

void MainWindow::tabChanged(TabType tabType) 
{
    QAction *readingList = KiwixApp::instance()->getAction(KiwixApp::ToggleReadingListAction);
    if (tabType == TabType::SettingsTab) { 
        mp_ui->sideBar->hide();    
    } else if(tabType == TabType::LibraryTab) { 
        mp_ui->sideBar->setCurrentWidget(mp_ui->contentmanagerside);
        mp_ui->sideBar->show();
    } else { 
        readingListToggled(readingList->isChecked());
    }
}

TabBar* MainWindow::getTabBar()
{
    return mp_ui->tabBar;
}

TopWidget *MainWindow::getTopWidget()
{
    return mp_ui->mainToolBar;
}

void MainWindow::togglePrivateMode()
{
    auto app = KiwixApp::instance();
    bool privateMode = app->isPrivateMode();
    if (privateMode) {
        QMessageBox::information(this, tr("Private Mode"), tr("Private mode is already active."));
    } else {
        QMessageBox::information(this, tr("Private Mode"), tr("Activating private mode."));
        // Add logic to activate private mode here
    }
}
