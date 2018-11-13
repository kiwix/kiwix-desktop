#include "webview.h"

#include <QAction>
#include <QWebEngineProfile>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebPage(this));
    auto profile = page()->profile();
    auto app = KiwixApp::instance();
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::onUrlChanged);
}

WebView::~WebView()
{}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        return tabWidget->createNewTab(type==QWebEnginePage::WebBrowserTab);
    }
    return nullptr;
}

void WebView::onUrlChanged(const QUrl& url) {
    if (m_currentHost != url.host() ) {
        m_currentHost = url.host();
        emit zimIdChanged(m_currentHost);
        auto app = KiwixApp::instance();
        auto reader = app->getLibrary()->getReader(m_currentHost);
        std::string favicon, _mimetype;
        reader->getFavicon(favicon, _mimetype);
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)favicon.data(), favicon.size());
        m_icon = QIcon(pixmap);
        emit iconChanged(m_icon);
    }
}
