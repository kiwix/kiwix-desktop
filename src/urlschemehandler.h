#ifndef URLSCHEMEHANDLER_H
#define URLSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class UrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
	Q_OBJECT
public:
    UrlSchemeHandler();
    void requestStarted(QWebEngineUrlRequestJob *request);
private:
    void handleMetaRequest(QWebEngineUrlRequestJob *request);
    void handleContentRequest(QWebEngineUrlRequestJob *request);
    void handleSearchRequest(QWebEngineUrlRequestJob *request);
};

#endif // URLSCHEMEHANDLER_H
