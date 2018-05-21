#include "kiwixwebview.h"

#include <QWebEngineProfile>
#include <iostream>

KiwixWebView::KiwixWebView(QWidget *parent)
    : QWebEngineView(parent)
{
    auto profile = page()->profile();
    profile->installUrlSchemeHandler("zim", &schemeHandler);
    profile->setRequestInterceptor(&requestInterceptor);

}

KiwixWebView::~KiwixWebView()
{}
