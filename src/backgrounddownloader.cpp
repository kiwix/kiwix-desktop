#include "backgrounddownloader.h"

#include <iostream>
#include <QString>

BackgroundDownloader::BackgroundDownloader(kiwix::Downloader* downloader)
    :  mp_downloader(downloader)
{
    m_thread.reset(new QThread); // no parent
    moveToThread(m_thread.get());
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

void BackgroundDownloader::startDownload(const QString& bookId, const QString& uri, const QString& downloadPath) {
    QMutexLocker ml(&m_mutex);

    kiwix::Download *download;
    try {
        std::pair<std::string, std::string> downloadDir("dir", downloadPath.toStdString());
        const std::vector<std::pair<std::string, std::string>> options = { downloadDir };
        download = mp_downloader->startDownload(uri.toStdString(), options);
        std::cerr << "got download with id " << download->getDid() << '\n';
        emit(confirmStartDownload(bookId, QString::fromStdString(download->getDid())));
    } catch (std::exception& e) {
        // Download failed, ignore
        std::cerr << "got exception " << e.what() << '\n';
    }
}