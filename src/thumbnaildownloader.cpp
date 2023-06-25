#include "thumbnaildownloader.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QIcon>

ThumbnailDownloader::ThumbnailDownloader(QObject *parent)
{
    connect(this, &ThumbnailDownloader::oneThumbnailDownloaded, [=]() {
        if (m_urlPairList.size() != 0)
            downloadOnePair(m_urlPairList.takeFirst());
        else
            m_isDownloading = false;
    });
}

ThumbnailDownloader::~ThumbnailDownloader()
{
}

void ThumbnailDownloader::addDownload(QString url, QModelIndex index)
{
    m_urlPairList.append({index, url});
    if (!m_isDownloading)
        startDownload();
}

void ThumbnailDownloader::startDownload()
{
    if (m_urlPairList.size() == 0) {
        m_isDownloading = false;
        return;
    }
    m_isDownloading = true;
    downloadOnePair(m_urlPairList.takeFirst());
}

void ThumbnailDownloader::downloadOnePair(QPair<QModelIndex, QString> urlPair)
{
    QNetworkRequest req(urlPair.second);
    auto reply = manager.get(req);
    connect(reply, &QNetworkReply::finished, this, [=](){
        fileDownloaded(reply, urlPair);
    });
}

void ThumbnailDownloader::fileDownloaded(QNetworkReply *pReply, QPair<QModelIndex, QString> urlPair)
{
    auto downloadedData = pReply->readAll();
    emit oneThumbnailDownloaded(urlPair.first, urlPair.second, downloadedData);
    pReply->deleteLater();
}
