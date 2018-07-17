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
    auto urlString = url.toString();
    std::cout << " - " << urlString.toUtf8().constData() << std::endl;
    if (urlString.startsWith("http://")) {
      urlString.replace(0, 7, "zim://");
    }
    std::cout << " + " << urlString.toUtf8().constData() << std::endl;
    info.redirect(QUrl(urlString));
}

