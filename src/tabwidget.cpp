#include "tabwidget.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{

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
