#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include "library.h"
#include <kiwix/downloader.h>
#include <QDebug>

class Downloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int nbDownload READ getNbDownload NOTIFY downloadsChanged)
    Q_PROPERTY(QStringList downloadIds READ getDownloadIds NOTIFY downloadsChanged)
public:
    explicit Downloader(Library* library, QObject *parent = nullptr);
    virtual ~Downloader();
    QStringList getDownloadIds();

signals:
    void downloadsChanged();

public slots:
    QString downloadBook(const QString& id);
    QStringList updateDownloadInfos(QString id, const QStringList& keys);
    int getNbDownload();

private:
    Library* mp_library;
    kiwix::Downloader m_downloader;
};

#endif // DOWNLOADER_H
