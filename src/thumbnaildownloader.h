#ifndef THUMBNAILDOWNLOADER_H
#define THUMBNAILDOWNLOADER_H

#include <QObject>
#include <QQueue>
#include <QNetworkAccessManager>
#include <QIcon>
#include <QNetworkReply>
#include <QModelIndex>

class ThumbnailDownloader : public QObject
{
    Q_OBJECT

public:
    typedef QModelIndex ThumbnailId;
    typedef QPair<ThumbnailId, QString> ThumbnailInfo;

public:
    ThumbnailDownloader();
    ~ThumbnailDownloader();

    void addDownload(QString url, ThumbnailId index);
    void startDownload();
    void clearQueue() { m_downloadQueue.clear(); }

private:
    void downloadThumbnail(ThumbnailInfo thumbnailInfo);

signals:
    void oneThumbnailDownloaded(ThumbnailId, QString, QByteArray);

private:
    QQueue<ThumbnailInfo> m_downloadQueue;
    QNetworkAccessManager manager;
    bool m_isDownloading = false;

private slots:
    void fileDownloaded(QNetworkReply *pReply, ThumbnailInfo thumbnailInfo);

};

#endif // THUMBNAILDOWNLOADER_H
