#include "tabwidget.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setTabsClosable(true);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::closeTab);
    connect(this, &QTabWidget::currentChanged, this, &TabWidget::onCurrentChanged);
}

WebView* TabWidget::createNewTab(bool setCurrent)
{
    WebView* webView = new WebView();
    QObject::connect(webView, &WebView::titleChanged, this,
                     [=](const QString& str) { setTitleOf(webView, str); });
    QObject::connect(webView, &WebView::iconChanged, this,
                     [=](const QIcon& icon) { setIconOf(webView, icon);  });
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

void TabWidget::setTitleOf(WebView* webView, const QString& title)
{
    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        setTabText(indexOf(webView), url.path());
    } else {
        setTabText(indexOf(webView), title);
    }
}

void TabWidget::setIconOf(WebView *webView, const QIcon &icon)
{
    setTabIcon(indexOf(webView), icon);
}

void TabWidget::triggerWebPageAction(QWebEnginePage::WebAction action)
{
    if (auto webView = currentWidget()) {
        webView->triggerPageAction(action);
        webView->setFocus();
    }
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
