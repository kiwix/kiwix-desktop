#ifndef KIWIXREQUESTINTERCEPTOR_H
#define KIWIXREQUESTINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>


class KiwixRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    KiwixRequestInterceptor();
    virtual void interceptRequest(QWebEngineUrlRequestInfo &info);
};

#endif // KIWIXREQUESTINTERCEPTOR_H
