#include "downloadmanagement.h"

#include "kiwixapp.h"
#include "kiwixmessagebox.h"

#include <QStorageInfo>
#include <QThread>

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

DownloadState::Status getDownloadStatus(QString status)
{
    if ( status == "active" )    return DownloadState::DOWNLOADING;
    if ( status == "paused" )    return DownloadState::PAUSED;
    if ( status == "waiting" )   return DownloadState::WAITING;
    if ( status == "error" )     return DownloadState::DOWNLOAD_ERROR;
    return DownloadState::UNKNOWN;
}

QString formatTimeRemaining(double seconds)
{
    int totalSec = int(seconds);
    int hours = totalSec / 3600;
    int minutes = (totalSec % 3600) / 60;
    int secs = totalSec % 60;
    if (hours > 0)
        return QString("%1:%2:%3")
                   .arg(hours, 2, 10, QChar('0'))
                   .arg(minutes, 2, 10, QChar('0'))
                   .arg(secs, 2, 10, QChar('0'));
    else
        return QString("%1:%2")
                   .arg(minutes, 2, 10, QChar('0'))
                   .arg(secs, 2, 10, QChar('0'));
}

} // unnamed namespace

bool DownloadState::isLateUpdateInfo(const DownloadInfo& info) const
{
    if ( ! stateChangeHasBeenRequested() )
       return false;

    const auto updateRequestTime = info["updateRequestTime"].toDouble();
    return updateRequestTime < lastUpdated.time_since_epoch().count();
}

void DownloadState::update(const DownloadInfo& info)
{
    const auto completedBytes = info["completedLength"].toDouble();
    const double totalBytes = info["totalLength"].toDouble();
    const double percentage = completedBytes / totalBytes;

    progress = QString::number(100 * percentage, 'g', 3).toDouble();
    completedLength = convertToUnits(completedBytes);
    double speedVal = info["downloadSpeed"].toDouble();
    downloadSpeed = convertToUnits(speedVal) + "/s";
    // Compute estimated time remaining
    double remainingSeconds = (speedVal > 0) ? ((totalBytes - completedBytes) / speedVal) : 0;
    estimatedTimeRemaining = (speedVal > 0) ? formatTimeRemaining(remainingSeconds)
                                            : "---";
    if ( !isLateUpdateInfo(info) ) {
        status = getDownloadStatus(info["status"].toString());
    }
    lastUpdated = std::chrono::steady_clock::now();
}

double DownloadState::timeSinceLastUpdate() const
{
    typedef std::chrono::duration<double> Seconds;

    const auto dt = std::chrono::steady_clock::now() - lastUpdated;
    return std::chrono::duration_cast<Seconds>(dt).count();
}

QString DownloadState::getDownloadSpeed() const
{
    return timeSinceLastUpdate() > 2.0 ? "---" : downloadSpeed;
}

void DownloadState::changeState(Action action)
{
    const auto oldStatus = status;
    if ( action == PAUSE ) {
        if ( status == DOWNLOADING ) {
            status = PAUSE_REQUESTED;
        }
    } else if ( action == RESUME ) {
        if ( status == PAUSED ) {
            status = RESUME_REQUESTED;
        }
    } else if ( action == CANCEL ) {
        if ( status == DOWNLOADING || status == PAUSED ) {
            status = CANCEL_REQUESTED;
        }
    }

    if ( status != oldStatus ) {
        lastUpdated = std::chrono::steady_clock::now();
    }
}

////////////////////////////////////////////////////////////////////////////////
// DowloadManager
////////////////////////////////////////////////////////////////////////////////

namespace
{

kiwix::Downloader* createDownloader()
{
    try {
        return new kiwix::Downloader(getDataDirectory().toStdString());
    } catch (std::exception& e) {
        QMessageBox::critical(nullptr, gt("error-downloader-window-title"),
        gt("error-downloader-launch-message") + "<br><br>" + e.what());
        return nullptr;
    }
}

} // unnamed namespace

DownloadManager::DownloadManager(const Library* lib)
    : mp_library(lib)
    , mp_downloader(createDownloader())
{
    restoreDownloads();
}

DownloadManager::~DownloadManager()
{
    if ( mp_downloadUpdaterThread )
    {
        QThread* t = mp_downloadUpdaterThread;
        mp_downloadUpdaterThread = nullptr; // tell the thread to terminate

        // At this point the thread may be stuck waiting for data.
        // Let's wake it up.
        m_requestQueue.enqueue({DownloadState::UPDATE, ""});
        t->wait();
    }
}

bool DownloadManager::downloadingFunctionalityAvailable() const
{
    return mp_downloader.get() != nullptr;
}

void DownloadManager::processDownloadActions()
{
   while ( mp_downloadUpdaterThread != nullptr ) {
        const Request req = m_requestQueue.dequeue();
        if ( !req.bookId.isEmpty() ) {
            switch ( req.action ) {
            case DownloadState::START:  startDownload(req.bookId);  break;
            case DownloadState::PAUSE:  pauseDownload(req.bookId);  break;
            case DownloadState::RESUME: resumeDownload(req.bookId); break;
            case DownloadState::CANCEL: cancelDownload(req.bookId); break;
            case DownloadState::UPDATE: updateDownload(req.bookId); break;
            }
        }
    }
}

void DownloadManager::startDownloadUpdaterThread()
{
    // so that DownloadInfo can be copied across threads
    qRegisterMetaType<DownloadInfo>("DownloadInfo");

    mp_downloadUpdaterThread = QThread::create([=]() {
        processDownloadActions();
    });

    mp_downloadUpdaterThread->start();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        if ( m_requestQueue.isEmpty() ) {
            for ( const auto& bookId : m_downloads.keys() ) {
                addRequest(DownloadState::UPDATE, bookId);
            }
        }
    });
    timer->start(1000);
}

void DownloadManager::restoreDownloads()
{
    for ( const auto& bookId : mp_library->getBookIds() ) {
        const kiwix::Book& book = mp_library->getBookById(bookId);
        if ( ! book.getDownloadId().empty() ) {
            const auto newDownload = std::make_shared<DownloadState>();
            m_downloads.set(bookId, newDownload);
        }
    }
}

void DownloadManager::updateDownload(QString bookId)
{
    const auto updateRequestTime = std::chrono::steady_clock::now();
    DownloadInfo downloadInfo;
    try {
        downloadInfo = getDownloadInfo(bookId);
    } catch ( ... ) {
        emit downloadDisappeared(bookId);
        return;
    }

    downloadInfo["updateRequestTime"] = double(updateRequestTime.time_since_epoch().count());
    emit downloadUpdated(bookId, downloadInfo);
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

    // Calculate estimated time remaining.
    double total = d->getTotalLength();
    double completed = d->getCompletedLength();
    double speed = d->getDownloadSpeed(); // bytes per second
    double remainingSeconds = (speed > 0) ? ((total - completed) / speed) : 0;
    QString timeRemaining = (speed > 0) ? formatTimeRemaining(remainingSeconds) : "---";

    return {
             { "status"              , downloadStatus2QString(d->getStatus())   },
             { "completedLength"     , QString::number(d->getCompletedLength()) },
             { "totalLength"         , QString::number(d->getTotalLength())     },
             { "downloadSpeed"       , QString::number(d->getDownloadSpeed())   },
             { "estimatedTimeRemaining", timeRemaining },
             { "path"                , QString::fromStdString(d->getPath())     }
    };
}

namespace
{

void throwDownloadUnavailableError()
{
    throw KiwixAppError(gt("download-unavailable"),
                        gt("download-unavailable-text"));
}

uint64_t getMaxFileSize(const QStorageInfo& storageInfo)
{
    const uint64_t GiB = 1024UL*1024UL*1024UL;

    // Maximum file size limits of filesystems that are expected to be
    // encountered in practice and cause problems should be entered below
    static const QMap<QByteArray, uint64_t> problematicFileSystems = {
// The output of QStorageInfo::fileSystemType() is platform dependent
#ifdef _WIN32
        // XXX: find out the real name for FAT32 as reported under Windows
        { "FAT32", 4*GiB - 1 }
#else
        { "vfat",  4*GiB - 1 }
#endif
    };

    const uint64_t virtuallyNoLimit = std::numeric_limits<uint64_t>::max();
    const auto fsType = storageInfo.fileSystemType();
    return problematicFileSystems.value(fsType, virtuallyNoLimit);
}

void checkThatBookCanBeSaved(const kiwix::Book& book, QString targetDir)
{
    const QFileInfo targetDirInfo(targetDir);
    if ( !targetDirInfo.isDir() ) {
        throw KiwixAppError(gt("download-storage-error"),
                            gt("download-dir-missing"));
    }

    // XXX: This may lie under Windows
    // XXX: (see https://doc.qt.io/qt-5/qfile.html#platform-specific-issues)
    if ( !targetDirInfo.isWritable() ) {
        throw KiwixAppError(gt("download-storage-error"),
                            gt("download-dir-not-writable"));
    }

    QStorageInfo storage(targetDir);
    auto bytesAvailable = storage.bytesAvailable();
    if (bytesAvailable == -1 || book.getSize() > (unsigned long long) bytesAvailable) {
        throw KiwixAppError(gt("download-storage-error"),
                            gt("download-storage-error-text"));
    }

    if ( book.getSize() > getMaxFileSize(storage) ) {
        throw KiwixAppError(gt("download-storage-error"),
                            gt("download-exceeds-max-file-size"));
    }
}

} // unnamed namespace


void DownloadManager::checkThatBookCanBeDownloaded(const kiwix::Book& book, const QString& downloadDirPath)
{
    if ( ! DownloadManager::downloadingFunctionalityAvailable() )
        throwDownloadUnavailableError();

    checkThatBookCanBeSaved(book, downloadDirPath);
}

std::string DownloadManager::startDownload(const kiwix::Book& book, const QString& downloadDirPath)
{
    const std::string& url = book.getUrl();
    const QString bookId = QString::fromStdString(book.getId());

    std::string downloadId;
    try {
        const auto d = mp_downloader->startDownload(url, downloadDirPath.toStdString());
        downloadId = d->getDid();
    } catch (std::exception& e) {
        throwDownloadUnavailableError();
    }

    return downloadId;
}

void DownloadManager::addRequest(Action action, QString bookId)
{
    if ( action == DownloadState::START ) {
        m_downloads.set(bookId, std::make_shared<DownloadState>());
    }

    if ( const auto downloadState = getDownloadState(bookId) ) {
        m_requestQueue.enqueue({action, bookId});
        if ( action != DownloadState::UPDATE ) {
            downloadState->changeState(action);
        }
    }
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

void DownloadManager::cancelDownload(const QString& bookId)
{
    const auto downloadId = mp_library->getBookById(bookId).getDownloadId();
    if ( downloadId.empty() ) {
        // Completion of the download has been detected (and its id was reset)
        // before the confirmation to cancel the download was granted.
        return;
    }

    auto download = mp_downloader->getDownload(downloadId);
    try {
        download->cancelDownload();
    } catch (const kiwix::AriaError&) {
        // Download has completed before the cancel request was handled.
        // Most likely the download was already complete at the time
        // when ContentManager::reallyCancelBook() started executing, but
        // its completion was not yet detected (and/or handled) by the
        // download updater thread (letting the code pass past the empty
        // downloadId check above).
        return;
    }
    emit downloadCancelled(bookId);
}

void DownloadManager::removeDownload(QString bookId)
{
    m_downloads.remove(bookId);
}
