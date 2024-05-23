#include "downloadmanagement.h"

////////////////////////////////////////////////////////////////////////////////
// DowloadState
////////////////////////////////////////////////////////////////////////////////

namespace
{

QString convertToUnits(double bytes)
{
    QStringList units = {"bytes", "KB", "MB", "GB", "TB", "PB", "EB"};
    int unitIndex = 0;
    while (bytes >= 1024 && unitIndex < units.size()) {
        bytes /= 1024;
        unitIndex++;
    }

    const auto preciseBytes = QString::number(bytes, 'g', 3);
    return preciseBytes + " " + units[unitIndex];
}

} // unnamed namespace

void DownloadState::update(const DownloadInfo& downloadInfos)
{
    double percent = downloadInfos["completedLength"].toDouble() / downloadInfos["totalLength"].toDouble();
    percent *= 100;
    percent = QString::number(percent, 'g', 3).toDouble();
    auto completedLength = convertToUnits(downloadInfos["completedLength"].toDouble());
    auto downloadSpeed = convertToUnits(downloadInfos["downloadSpeed"].toDouble()) + "/s";
    const bool paused = downloadInfos["status"] == "paused";
    *this = {percent, completedLength, downloadSpeed, paused};
}

////////////////////////////////////////////////////////////////////////////////
// DowloadManager
////////////////////////////////////////////////////////////////////////////////

DownloadManager::DownloadManager(const Library* lib, kiwix::Downloader *downloader)
    : mp_library(lib)
    , mp_downloader(downloader)
{
    restoreDownloads();
}

void DownloadManager::restoreDownloads()
{
    for ( const auto& bookId : mp_library->getBookIds() ) {
        const kiwix::Book& book = mp_library->getBookById(bookId);
        if ( ! book.getDownloadId().empty() ) {
            const auto newDownload = std::make_shared<DownloadState>();
            newDownload->paused = true;
            m_downloads.set(bookId, newDownload);
        }
    }
}

void DownloadManager::updateDownloads()
{
    DownloadInfo downloadInfo;
    for ( const auto& bookId : m_downloads.keys() ) {
        try {
            downloadInfo = getDownloadInfo(bookId);
        } catch ( ... ) {
            emit downloadDisappeared(bookId);
            continue;
        }

        emit downloadUpdated(bookId, downloadInfo);
    }
}

namespace
{

QString downloadStatus2QString(kiwix::Download::StatusResult status)
{
    switch(status){
    case kiwix::Download::K_ACTIVE:   return "active";
    case kiwix::Download::K_WAITING:  return "waiting";
    case kiwix::Download::K_PAUSED:   return "paused";
    case kiwix::Download::K_ERROR:    return "error";
    case kiwix::Download::K_COMPLETE: return "completed";
    case kiwix::Download::K_REMOVED:  return "removed";
    default:                          return "unknown";
    }
}

} // unnamed namespace

DownloadInfo DownloadManager::getDownloadInfo(QString bookId) const
{
    auto& b = mp_library->getBookById(bookId);
    const auto d = mp_downloader->getDownload(b.getDownloadId());
    d->updateStatus(true);

    return {
             { "status"          , downloadStatus2QString(d->getStatus())   },
             { "completedLength" , QString::number(d->getCompletedLength()) },
             { "totalLength"     , QString::number(d->getTotalLength())     },
             { "downloadSpeed"   , QString::number(d->getDownloadSpeed())   },
             { "path"            , QString::fromStdString(d->getPath())     }
    };
}

std::string DownloadManager::startDownload(const std::string& url, const std::string& downloadDirPath)
{
    typedef std::vector<std::pair<std::string, std::string>> DownloadOptions;

    const DownloadOptions downloadOptions{{"dir", downloadDirPath}};

    const auto d = mp_downloader->startDownload(url, downloadOptions);
    return d->getDid();
}

void DownloadManager::pauseDownload(const QString& bookId)
{
    const auto downloadId = mp_library->getBookById(bookId).getDownloadId();
    if ( downloadId.empty() ) {
        // Completion of the download has been detected (and its id was reset)
        // before the pause-download action was triggered (most likely through
        // the context menu which can stay open for an arbitrarily long time,
        // or, unlikely, through the ⏸ button during the last milliseconds of
        // the download progress).
        return;
    }

    auto download = mp_downloader->getDownload(downloadId);
    if (download->getStatus() == kiwix::Download::K_ACTIVE) {
        try {
            download->pauseDownload();
        } catch (const kiwix::AriaError&) {
            // Download has completed before the pause request was handled.
            // Most likely the download was already complete at the time
            // when ContentManager::pauseBook() started executing, but its
            // completion was not yet detected (and/or handled) by the download
            // updater thread.
        }
    }
}

void DownloadManager::resumeDownload(const QString& bookId)
{
    auto& b = mp_library->getBookById(bookId);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() == kiwix::Download::K_PAUSED) {
        download->resumeDownload();
    }
}
