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
    typedef std::pair<QString, bool> DownloadInfo;
    
public:
    KProfile(QObject *parent = nullptr);
    void setFileOpenChoice(QString fileName, bool choice);
    DownloadInfo getFileOpenChoice() const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QString getFileName(QWebEngineDownloadItem* download);
#else
    QString getFileName(QWebEngineDownloadRequest* download);
#endif
    bool isNonReadableFile(QString mimeType);
    

private:
    UrlSchemeHandler m_schemeHandler;
    DownloadInfo m_fileOpenChoice;

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