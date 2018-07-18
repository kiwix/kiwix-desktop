#include "ktabwidget.h"

KTabWidget::KTabWidget(QWidget *parent) :
    QTabWidget(parent)
{

}

KiwixWebView* KTabWidget::createNewTab(bool setCurrent)
{
    KiwixWebView* webView = new KiwixWebView();
    QObject::connect(webView, &KiwixWebView::titleChanged, this,
                     [=](const QString& str) { setTitleOf(webView, str); });
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

void KTabWidget::setTitleOf(KiwixWebView* webView, const QString& title)
{
    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        setTabText(indexOf(webView), url.path());
    } else {
        setTabText(indexOf(webView), title);
    }
}
