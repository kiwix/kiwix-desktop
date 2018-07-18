#include "ktabwidget.h"

KTabWidget::KTabWidget(QWidget *parent) :
    QTabWidget(parent)
{

}

KiwixWebView* KTabWidget::createNewTab(bool setCurrent)
{
    KiwixWebView* webView = new KiwixWebView();
    // Ownership of webview is passed to the tabWidget
    addTab(webView, "");
    if (setCurrent) {
        setCurrentWidget(webView);
    }
    return webView;
}

void KTabWidget::openUrl(std::shared_ptr<kiwix::Reader> reader, const QUrl& url, bool newTab)
{
    KiwixWebView* webView = nullptr;
    if (newTab || !currentWidget()) {
        webView = createNewTab(true);
    }
    webView->setUrl(url);
}
