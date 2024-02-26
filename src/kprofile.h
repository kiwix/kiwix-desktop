#ifndef KPROFILE_H
#define KPROFILE_H

#include <QWebEngineProfile>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QWebEngineDownloadItem>
#else
#include <QWebEngineDownloadRequest>
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
public slots:

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void startDownload(QWebEngineDownloadItem*);
#else
    void startDownload(QWebEngineDownloadRequest*);
#endif
    void downloadFinished();
};

#endif // KPROFILE_H
