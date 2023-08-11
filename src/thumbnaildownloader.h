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
    ThumbnailDownloader(QObject *parent = 0);
    ~ThumbnailDownloader();

    void addDownload(QString url, QModelIndex index);
    void startDownload();
    void downloadOnePair(QPair<QModelIndex, QString> urlPair);
    void clearQueue() { m_urlPairList.clear(); }

signals:
    void oneThumbnailDownloaded(QModelIndex, QString, QByteArray);

private:
    QQueue<QPair<QModelIndex, QString>> m_urlPairList;
    QNetworkAccessManager manager;
    bool m_isDownloading = false;

private slots:
    void fileDownloaded(QNetworkReply *pReply, QPair<QModelIndex, QString> urlPair);

};

#endif // THUMBNAILDOWNLOADER_H
