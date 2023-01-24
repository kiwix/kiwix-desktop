#include "backgrounddownloader.h"

#include <iostream>
#include <QString>
#include <QTimer>
#include <QDebug>

BackgroundDownloader::BackgroundDownloader(kiwix::Downloader* downloader)
    :  mp_downloader(downloader)
{
    QThread* newThread = new QThread(); // no parent
    m_thread.reset(newThread);
    moveToThread(m_thread.get());

    // the timer causes updateStatus() to be called once per second on this thread's event loop
    mp_timer = new QTimer(newThread);
    connect(mp_timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    mp_timer->start(1000);

    m_thread->start();
}

BackgroundDownloader::~BackgroundDownloader() {
    QMetaObject::invokeMethod(this, "cleanup");
    m_thread->wait();
}

void BackgroundDownloader::cleanup() {
    // TODO: delete resources

    m_thread->quit();
}

// getDownloadStatus returns the status map for this downloadId. May be called from a separate thread
QMap<std::string, std::string> BackgroundDownloader::getDownloadStatus(const std::string did)
{
    m_rwlock.lockForRead();
    QMap<std::string, std::string> map = m_status.value(did);
    m_rwlock.unlock();
    return map;
}

void BackgroundDownloader::startDownload(const QString& bookId, const QString& uri, const QString& downloadPath)
{
    kiwix::Download *download;
    try {
        std::pair<std::string, std::string> downloadDir("dir", downloadPath.toStdString());
        const std::vector<std::pair<std::string, std::string>> options = { downloadDir };
        m_rwlock.lockForWrite();
        download = mp_downloader->startDownload(uri.toStdString(), options);
        m_rwlock.unlock();
        std::cerr << "got download with id " << download->getDid() << '\n';
        emit(confirmStartDownload(bookId, QString::fromStdString(download->getDid())));
    } catch (std::exception& e) {
        // Download failed, ignore
        std::cerr << "got exception " << e.what() << '\n';
    }
}

// Cancel the download if it is running, then remove from the m_status map
void BackgroundDownloader::completeDownload(const QString& did)
{
    kiwix::Download* download = mp_downloader->getDownload(did.toStdString());

    m_rwlock.lockForWrite();
    QMap<std::string, std::string> map = m_status.value(did.toStdString());
    m_status.remove(did.toStdString());
    download->cancelDownload();
    m_rwlock.unlock();
}

void BackgroundDownloader::pauseDownload(const QString& did)
{
    kiwix::Download* download = mp_downloader->getDownload(did.toStdString());
    if (download->getStatus() == kiwix::Download::K_ACTIVE) {
        m_rwlock.lockForWrite();
        download->pauseDownload();
        m_rwlock.unlock();
    }
}

void BackgroundDownloader::resumeDownload(const QString& did)
{
    kiwix::Download* download = mp_downloader->getDownload(did.toStdString());
    if (download->getStatus() == kiwix::Download::K_PAUSED) {
        m_rwlock.lockForWrite();
        download->resumeDownload();
        m_rwlock.unlock();
    }
}

void BackgroundDownloader::cancelDownload(const QString& bookId, const QString& did)
{
    qInfo() << "canceling download for book = " << bookId;
    kiwix::Download* download = mp_downloader->getDownload(did.toStdString());

    m_rwlock.lockForWrite();
    QMap<std::string, std::string> map = m_status.value(did.toStdString());
    m_status.remove(did.toStdString());
    download->cancelDownload();
    m_rwlock.unlock();

    emit(confirmCancelDownload(bookId, QString::fromStdString(map.value("path"))));
}

void BackgroundDownloader::updateStatus()
{
    for (auto& did : mp_downloader->getDownloadIds()) {
        qInfo() << "updating status for did " << did.c_str();
        kiwix::Download* download = mp_downloader->getDownload(did);
        download->updateStatus(true);

        m_rwlock.lockForWrite();
        auto status = m_status.value(did);
        status.insert("id", download->getDid());
        status.insert("followed_by", download->getFollowedBy());
        status.insert("path", download->getPath());
        status.insert("totalLength", std::to_string(download->getTotalLength()));
        status.insert("completedLength", std::to_string(download->getCompletedLength()));
        status.insert("downloadSpeed", std::to_string(download->getDownloadSpeed()));
        status.insert("verifiedLength", std::to_string(download->getVerifiedLength()));
        status.insert("status", convertStatusResult(download->getStatus()));
        m_status.insert(did, status);
        m_rwlock.unlock();
    }
    int mapSize = 0;
    for (auto it = m_status.begin(); it != m_status.end(); ++it) {
        mapSize++;
    }

    qInfo() << "finished updating status with downloads length = " << mapSize;
}

std::string BackgroundDownloader::convertStatusResult(kiwix::Download::StatusResult result) {
    switch(result){
    case kiwix::Download::K_ACTIVE:
        return "active";
    case kiwix::Download::K_WAITING:
        return "waiting";
    case kiwix::Download::K_PAUSED:
        return "paused";
    case kiwix::Download::K_ERROR:
        return "error";
    case kiwix::Download::K_COMPLETE:
        return "completed";
    case kiwix::Download::K_REMOVED:
        return "removed";
    case kiwix::Download::K_UNKNOWN:
        return "unknown";
    }
    return "unknown";
}
