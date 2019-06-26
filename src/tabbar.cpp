#include "tabbar.h"

#include "kiwixapp.h"
#include <QAction>
#include <QTimer>
#include <QWebEnginePage>
#include <QToolButton>

#define QUITIFNULL(VIEW) if (nullptr==(VIEW)) { return; }
#define QUITIFNOTCURRENT(VIEW) if((VIEW)!=currentWidget()) {return;}
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentWidget();}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent)
{
    setTabsClosable(true);
    setElideMode(Qt::ElideRight);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    setIconSize(QSize(30, 30));
    connect(this, &QTabBar::tabCloseRequested, this, &TabBar::closeTab);
    connect(this, &QTabBar::currentChanged, this, &TabBar::onCurrentChanged);
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::NewTabAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                auto widget = this->createNewTab(true);
                QUITIFNULL(current);
                widget->setUrl(current->url());
          });
    connect(app->getAction(KiwixApp::CloseTabAction), &QAction::triggered,
            this, [=]() {
                auto index = this->currentIndex();
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
            });
    connect(app->getAction(KiwixApp::ZoomOutAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                auto zoomFactor = current->zoomFactor();
                zoomFactor -= 0.1;
                zoomFactor = max(min(zoomFactor, 5.0), 0.25);
                current->setZoomFactor(zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomResetAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                current->setZoomFactor(1.0);
            });
    connect(app->getAction(KiwixApp::OpenHomePageAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWidget();
                QUITIFNULL(current);
                current->setUrl("zim://" + current->zimId() + ".zim/");
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
    tb->setText("+");
    addTab("");
    setTabEnabled(1, false);
    setTabButton(1, QTabBar::LeftSide, tb);
    tabButton(1, QTabBar::RightSide)->deleteLater();
    setTabButton(1, QTabBar::RightSide, 0);
}

WebView* TabBar::createNewTab(bool setCurrent)
{
    WebView* webView = new WebView();
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
    // Ownership of webview is passed to the tabWidget
    auto index = count() - 1;
    mp_stackedWidget->insertWidget(index, webView);
    insertTab(index, "");
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
    if (index == 0)
        return;
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
    if (index == count() - 2)
    {
        QTabBar::setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    } else {
        QTabBar::setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
    }
}

void TabBar::onCurrentChanged(int index)
{
    if (index == -1)
        return;
    if (index)
    {
        auto view = widget(index);
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        KiwixApp::instance()->setSideBar(KiwixApp::NONE);
        QTimer::singleShot(0, [=](){emit currentTitleChanged(view->title());});
    } else {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        KiwixApp::instance()->setSideBar(KiwixApp::CONTENTMANAGER_BAR);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    }
}
