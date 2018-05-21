#include "kiwixrequestinterceptor.h"

#include <QWebEngineUrlRequestInfo>
#include <QDebug>
#include <iostream>

KiwixRequestInterceptor::KiwixRequestInterceptor()
{

}


void KiwixRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    std::cout << "Intercept request" << std::endl;
    auto url = info.requestUrl();
    std::cout << "  - " << url.toString().toUtf8().constData() << std::endl;
    url.setScheme("zim");
    std::cout << "  + " << url.toString().toUtf8().constData() << std::endl;
    info.redirect(url);

}

