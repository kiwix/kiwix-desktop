#ifndef URLSCHEMEHANDLER_H
#define URLSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class UrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    UrlSchemeHandler();
    void requestStarted(QWebEngineUrlRequestJob *request);
};

#endif // URLSCHEMEHANDLER_H
