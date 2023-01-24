#ifndef BACKGROUNDDOWNLOADER_H
#define BACKGROUNDDOWNLOADER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMap>
#include <QReadWriteLock>
#include <memory>
#include <kiwix/downloader.h>

class BackgroundDownloader : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundDownloader(kiwix::Downloader* downloader);
    virtual ~BackgroundDownloader();

    QMap<std::string, std::string> getDownloadStatus(const std::string did);

signals:
    void confirmStartDownload(const QString& bookID, const QString& did);
    void confirmCancelDownload(const QString& bookId, const QString& path);

public slots:
    void startDownload(const QString& bookId, const QString& uri, const QString& downloadDir);
    void completeDownload(const QString& did);
    void pauseDownload(const QString& did);
    void resumeDownload(const QString& did);
    void cancelDownload(const QString& bookId, const QString& did);

    void updateStatus();

private slots:
    void cleanup();

private:
    std::string convertStatusResult(kiwix::Download::StatusResult result);

    std::unique_ptr<QThread> m_thread;
    QReadWriteLock m_rwlock;
    QTimer* mp_timer;

    // m_status contains the status of all downloads by download ID
    QMap<std::string, QMap<std::string, std::string>> m_status;
    kiwix::Downloader* mp_downloader;
};

#endif // BACKGROUNDDOWNLOADER_H
