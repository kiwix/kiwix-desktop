#include "webview.h"
#include <QAction>
#include <QWebEngineProfile>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"

WebView::WebView(QWidget *parent)
    :QWebEngineView(parent)
{
    setPage(new WebPage(this));   //to show the ZIM file as a webpage
    auto profile = page()->profile();
    auto app = KiwixApp::instance();
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::onUrlChanged);
}

WebView::~WebView()                                     //gaussian max destructor
{}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)       //Function to create webview window
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();      //to apply the Tab Widget
        return tabWidget->createNewTab(type==QWebEnginePage::WebBrowserTab);
    }
    return nullptr;
}

void WebView::onUrlChanged(const QUrl& url)             //function to do an action in the webview when URL is changed. 
{
    auto zimId = url.host();
    zimId.resize(zimId.length()-4);
    if (m_currentZimId != zimId ) {
        m_currentZimId = zimId;             //to retrive the correct ZIM files
        emit zimIdChanged(m_currentZimId);
        auto app = KiwixApp::instance();
        auto reader = app->getLibrary()->getReader(m_currentZimId);  //reading the Zim files from the server
        std::string favicon, _mimetype;
        reader->getFavicon(favicon, _mimetype);     //setting the favicon icon
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)favicon.data(), favicon.size());
        m_icon = QIcon(pixmap);                 //pasting the favicon icon
        emit iconChanged(m_icon);
    }
}
