#include "tabwidget.h"

#define QUITIFNULL(VIEW) if (nullptr==(VIEW)) { return; }
#define QUITIFNOTCURRENT(VIEW) if((VIEW)!=currentWidget()) {return;}
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentWidget();}

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setTabsClosable(true);
    setElideMode(Qt::ElideNone);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::closeTab);
    connect(this, &QTabWidget::currentChanged, this, &TabWidget::onCurrentChanged);
}

WebView* TabWidget::createNewTab(bool setCurrent)
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
    addTab(webView, "");
    if (setCurrent) {
        setCurrentWidget(webView);
    }
    return webView;
}

void TabWidget::openUrl(const QUrl& url, bool newTab)
{
    WebView* webView = nullptr;
    if (newTab || !currentWidget()) {
        webView = createNewTab(true);
    }
    webView->setUrl(url);
}

void TabWidget::setTitleOf(const QString& title, WebView* webView)
{
    CURRENTIFNULL(webView);
    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        setTabText(indexOf(webView), url.path());
    } else {
        setTabText(indexOf(webView), title);
    }
}

void TabWidget::setIconOf(const QIcon &icon, WebView *webView)
{
    CURRENTIFNULL(webView);
    setTabIcon(indexOf(webView), icon);
}

const QString &TabWidget::currentZimId()
{
    return currentWidget()->zimId();
}

void TabWidget::triggerWebPageAction(QWebEnginePage::WebAction action, WebView *webView)
{
    CURRENTIFNULL(webView);
    QUITIFNULL(webView);
    webView->triggerPageAction(action);
    webView->setFocus();
}

void TabWidget::closeTab(int index)
{
    auto webview = widget(index);
    removeTab(index);
    webview->close();
    delete webview;
}

void TabWidget::onCurrentChanged(int index)
{
    if (index != 1)
    {
        auto view = widget(index);
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
    }
}
