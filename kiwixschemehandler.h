#ifndef KIWIXSCHEMEHANDLER_H
#define KIWIXSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class KiwixSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    KiwixSchemeHandler();
    void requestStarted(QWebEngineUrlRequestJob *request);
};

#endif // KIWIXSCHEMEHANDLER_H
