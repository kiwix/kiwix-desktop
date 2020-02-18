#include "tabbar.h"

#include "kiwixapp.h"
#include <QAction>
#include <QTimer>
#include <QWebEnginePage>
#include <QToolButton>
#include <QToolTip>
#include <QCursor>

#define QUITIFNULL(VIEW) if (nullptr==(VIEW)) { return; }
#define QUITIFNOTCURRENT(VIEW) if((VIEW)!=currentWidget()) {return;}
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentWidget();}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent),
    m_settingsIndex(-1)
{
    setTabsClosable(true);
    setElideMode(Qt::ElideRight);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    setIconSize(QSize(30, 30));
    connect(this, &QTabBar::currentChanged, this, &TabBar::onCurrentChanged);
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::NewTabAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                auto widget = this->createNewTab(true);
                QUITIFNULL(current);
                KiwixApp::instance()->getMainWindow()->getTopWidget()->getSearchBar().setFocus(Qt::MouseFocusReason);
          });
    connect(app->getAction(KiwixApp::CloseTabAction), &QAction::triggered,
            this, [=]() {
                auto index = this->tabAt(mapFromGlobal(QCursor::pos()));
                if (index <= 0) {
                    return;
                }
                this->closeTab(index);
            });
    connect(app->getAction(KiwixApp::ZoomInAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                auto zoomFactor = current->zoomFactor();
                zoomFactor += 0.1;
                zoomFactor = max(min(zoomFactor, 5.0), 0.25);
                current->setZoomFactor(zoomFactor);
                auto key = this->currentZimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomOutAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                auto zoomFactor = current->zoomFactor();
                zoomFactor -= 0.1;
                zoomFactor = max(min(zoomFactor, 5.0), 0.25);
                current->setZoomFactor(zoomFactor);
                auto key = this->currentZimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomResetAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                auto settingsManager = KiwixApp::instance()->getSettingsManager();
                current->setZoomFactor(settingsManager->getZoomFactor());
                auto key = this->currentZimId() + "/zoomFactor";
                settingsManager->deleteSettings(key);
            });
    connect(app->getAction(KiwixApp::OpenHomePageAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                current->setUrl("zim://" + current->zimId() + ".zim/");
            });
    connect(app->getAction(KiwixApp::SettingAction), &QAction::triggered,
            this, [=]() {
                if (KiwixApp::instance()->getSettingsManager()->isSettingsViewdisplayed()) {
                    return;
                }
                auto index = currentIndex() + 1;
                auto view = KiwixApp::instance()->getSettingsManager()->getView();
                mp_stackedWidget->insertWidget(index, view);
                insertTab(index,QIcon(":/icons/settings.svg"), tr("Settings"));
                setCurrentIndex(index);
                m_settingsIndex = index;
            });
}

void TabBar::setStackedWidget(QStackedWidget *widget) {
    mp_stackedWidget = widget;
    connect(this, &QTabBar::currentChanged,
            widget, &QStackedWidget::setCurrentIndex);
}

void TabBar::setContentManagerView(ContentManagerView* view)
{
    qInfo() << "add widget";
    mp_contentManagerView = view;
    mp_stackedWidget->addWidget(mp_contentManagerView);
    mp_stackedWidget->show();
    addTab(QIcon(":/icons/kiwix/app_icon.svg"), "");
    setTabButton(0, RightSide, nullptr);
}

void TabBar::setNewTabButton()
{
    QToolButton *tb = new QToolButton();
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::NewTabAction));
    tb->setIcon(QIcon(":/icons/new-tab-icon.svg"));
    addTab("");
    setTabEnabled(1, false);
    setTabButton(1, QTabBar::LeftSide, tb);
    tabButton(1, QTabBar::RightSide)->deleteLater();
    setTabButton(1, QTabBar::RightSide, 0);
}

WebView* TabBar::createNewTab(bool setCurrent)
{
    WebView* webView = new WebView();
    connect(webView->page(), &QWebEnginePage::fullScreenRequested, this, &TabBar::fullScreenRequested);
    connect(webView, &WebView::titleChanged, this,
            [=](const QString& str) {
        setTitleOf(str, webView);
        QUITIFNOTCURRENT(webView);
        emit currentTitleChanged(str);
    });
    connect(webView, &WebView::iconChanged, this,
            [=](const QIcon& icon) { setIconOf(icon, webView);  });
    connect(webView, &WebView::zimIdChanged, this,
            [=](const QString& zimId) {
                QUITIFNOTCURRENT(webView);
                emit currentZimIdChanged(zimId);
            });
    connect(webView->page()->action(QWebEnginePage::Back), &QAction::changed,
            [=]() {
                QUITIFNOTCURRENT(webView);
                emit webActionEnabledChanged(QWebEnginePage::Back, webView->isWebActionEnabled(QWebEnginePage::Back));
            });
    connect(webView->page()->action(QWebEnginePage::Forward), &QAction::changed,
            [=]() {
                QUITIFNOTCURRENT(webView);
                emit webActionEnabledChanged(QWebEnginePage::Forward, webView->isWebActionEnabled(QWebEnginePage::Forward));
            });
    connect(webView->page(), &QWebEnginePage::linkHovered, this,
            [=](const QString& url) {
                auto tabbar = KiwixApp::instance()->getTabWidget();
                if (url.isEmpty()) {
                    QToolTip::hideText();
                } else {
                    auto link = url;
                    if (url.startsWith("zim://")) {
                        link = QUrl(url).path();
                    }
                    auto height = tabbar->currentWidget()->height() + 1;
                    auto pos = tabbar->mapToGlobal(QPoint(-3, height));
                    QToolTip::showText(pos, link);
                }
            });
    // Ownership of webview is passed to the tabWidget
    auto index = count() - 1;
    mp_stackedWidget->insertWidget(index, webView);
    insertTab(index, "");
    QToolButton *tb = new QToolButton(this);
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::CloseTabAction));
    setTabButton(index, QTabBar::RightSide, tb);
    if (setCurrent) {
        setCurrentIndex(index);
    }
    return webView;
}

void TabBar::openUrl(const QUrl& url, bool newTab)
{
    WebView* webView = currentWidget();
    if (newTab || !webView) {
        webView = createNewTab(true);
    }
    QUITIFNULL(webView);
    webView->setUrl(url);
}

void TabBar::setTitleOf(const QString& title, WebView* webView)
{
    CURRENTIFNULL(webView);
    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        setTabText(mp_stackedWidget->indexOf(webView), url.path());
    } else {
        setTabText(mp_stackedWidget->indexOf(webView), title);
    }
}

void TabBar::setIconOf(const QIcon &icon, WebView *webView)
{
    CURRENTIFNULL(webView);
    setTabIcon(mp_stackedWidget->indexOf(webView), icon);
}

QString TabBar::currentZimId()
{
    if (!currentWidget())
        return "";
    return currentWidget()->zimId();
}

QString TabBar::currentArticleUrl()
{
    if(!currentWidget())
        return "";
    return currentWidget()->url().path();
}

QString TabBar::currentArticleTitle()
{
    if(!currentWidget())
        return "";
    return currentWidget()->title();
}

QSize TabBar::tabSizeHint(int index) const {
    if (index)
        return QSize(205, 40);
    return QSize(40, 40);
}

void TabBar::triggerWebPageAction(QWebEnginePage::WebAction action, WebView *webView)
{
    CURRENTIFNULL(webView);
    QUITIFNULL(webView);
    webView->triggerPageAction(action);
    webView->setFocus();
}

void TabBar::closeTab(int index)
{
    if (index == 0 || index == this->count() - 1)
        return;
    if (index == m_settingsIndex) {
        m_settingsIndex = -1;
    }
    if (index < m_settingsIndex) {
        m_settingsIndex--;
    }
    setSelectionBehaviorOnRemove(index);
    auto webview = widget(index);
    mp_stackedWidget->removeWidget(webview);
    webview->setParent(nullptr);
    removeTab(index);
    webview->close();
    delete webview;
}

void TabBar::setSelectionBehaviorOnRemove(int index)
{
    if (index == count() - 2) {
        setCurrentIndex(index - 1);
    } else {
        setCurrentIndex(index + 1);
    }
}

void TabBar::onCurrentChanged(int index)
{
    if (index == -1)
        return;
    if (index == m_settingsIndex) {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit libraryPageDisplayed(false);
        KiwixApp::instance()->setSideBar(KiwixApp::NONE);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    } else if (index) {
        auto view = widget(index);
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        emit libraryPageDisplayed(false);
        if (KiwixApp::instance()->getSideType() == KiwixApp::CONTENTMANAGER_BAR) {
            KiwixApp::instance()->setSideBar(KiwixApp::NONE);
        }
        QTimer::singleShot(0, [=](){emit currentTitleChanged(view->title());});
    } else {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit libraryPageDisplayed(true);
        KiwixApp::instance()->setSideBar(KiwixApp::CONTENTMANAGER_BAR);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    }
}

void TabBar::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        if (m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset(new FullScreenWindow(this->currentWidget()));
    } else {
        if (!m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset();
    }
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        closeTab(this->tabAt(event->pos()));
    } else {
       QTabBar::mousePressEvent(event);
    }
}