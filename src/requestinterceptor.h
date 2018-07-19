#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>


class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    RequestInterceptor();
    virtual void interceptRequest(QWebEngineUrlRequestInfo &info);
};

#endif // REQUESTINTERCEPTOR_H
