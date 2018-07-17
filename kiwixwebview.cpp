#include "kiwixwebview.h"

#include <QWebEngineProfile>
#include <iostream>
#include "kiwixapp.h"

KiwixWebView::KiwixWebView(QWidget *parent)
    : QWebEngineView(parent)
{
    auto profile = page()->profile();
    auto app = static_cast<KiwixApp*>(KiwixApp::instance());
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
    profile->setRequestInterceptor(app->getRequestInterceptor());
}

KiwixWebView::~KiwixWebView()
{}


void KiwixWebView::initFromReader(std::shared_ptr<kiwix::Reader> reader)
{
    std::string url("zim://");
    url += reader->getId();
    url += ".zim/";
    page()->setUrl(QUrl(QString::fromStdString(url)));
}
