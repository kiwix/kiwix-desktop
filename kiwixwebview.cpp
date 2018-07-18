#include "kiwixwebview.h"

#include <QWebEngineProfile>
#include <iostream>
#include "kiwixapp.h"

KiwixWebView::KiwixWebView(QWidget *parent)
    : QWebEngineView(parent)
{
    auto profile = page()->profile();
    auto app = KiwixApp::instance();
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
    profile->setRequestInterceptor(app->getRequestInterceptor());
}

KiwixWebView::~KiwixWebView()
{}

QWebEngineView* KiwixWebView::createWindow(QWebEnginePage::WebWindowType type)
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        return tabWidget->createNewTab(type==QWebEnginePage::WebBrowserTab);
    }
    return nullptr;
}

}
