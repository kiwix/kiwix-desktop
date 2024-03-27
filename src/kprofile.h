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
    typedef std::pair<QString, QString> DownloadInfo;
public:
    KProfile(QObject *parent = nullptr);
    void setDownloadInfo(const QString &fileName, const QString &mimeType);
    DownloadInfo getDownloadInfo() const;

private:
    UrlSchemeHandler m_schemeHandler;
    DownloadInfo m_downloadInfo;

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