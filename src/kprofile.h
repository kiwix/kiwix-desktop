#ifndef KPROFILE_H
#define KPROFILE_H

#include <QWebEngineProfile>
#include <QWebEngineUrlRequestInterceptor>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QWebEngineDownloadItem>
typedef QWebEngineDownloadItem WebEngineDownloadType;
#else
#include <QWebEngineDownloadRequest>
typedef QWebEngineDownloadRequest WebEngineDownloadType;
#endif

#include "urlschemehandler.h"

class KProfile : public QWebEngineProfile
{
    Q_OBJECT
public:
    KProfile(QObject *parent = nullptr);

private:
    UrlSchemeHandler m_schemeHandler;

signals:

private slots:
    void startDownload(WebEngineDownloadType*);
    void saveFile(WebEngineDownloadType*);
    void openFile(WebEngineDownloadType*);
};

/**
 * @brief Intercepts and blocks a request if it is not native to our zim file.
 * https://stackoverflow.com/questions/70721311/qwebview-disable-external-resources
 */
class ExternalReqInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit ExternalReqInterceptor(QObject *parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent)
    {
    }

protected:
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;
};

#endif // KPROFILE_H
