#ifndef KPROFILE_H
#define KPROFILE_H

#include <QWebEngineProfile>

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
    void startDownload(QWebEngineDownloadItem*);
    void downloadFinished();
};

#endif // KPROFILE_H
