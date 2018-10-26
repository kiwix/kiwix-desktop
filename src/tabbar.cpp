#include "tabbar.h"

#include "kiwixapp.h"
#include <QAction>

#define QUITIFNULL(VIEW) if (nullptr==(VIEW)) { return; }
#define QUITIFNOTCURRENT(VIEW) if((VIEW)!=currentWidget()) {return;}
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentWidget();}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent)
{
    setTabsClosable(true);
    setElideMode(Qt::ElideNone);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
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
                auto widget = mp_stackedWidget->widget(index);
                mp_stackedWidget->removeWidget(widget);
                widget->setParent(nullptr);
                delete widget;
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
    addTab("contentManager");
}

WebView* TabBar::createNewTab(bool setCurrent)
{
    WebView* webView = new WebView();
    connect(webView, &WebView::titleChanged, this,
            [=](const QString& str) { setTitleOf(str, webView); });
    connect(webView, &WebView::iconChanged, this,
            [=](const QIcon& icon) { setIconOf(icon, webView);  });
    connect(webView, &WebView::zimIdChanged, this,
            [=](const QString& zimId) {
                QUITIFNOTCURRENT(webView);
                emit currentZimIdChanged(zimId);
            }
    );
    // Ownership of webview is passed to the tabWidget
    mp_stackedWidget->addWidget(webView);
    auto index = addTab("");
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
    auto webview = widget(index);
    removeTab(index);
    webview->close();
    delete webview;
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
    } else {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        KiwixApp::instance()->setSideBar(KiwixApp::CONTENTMANAGER_BAR);
    }
}
