#ifndef BACKGROUNDDOWNLOADER_H
#define BACKGROUNDDOWNLOADER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <memory>
#include <kiwix/downloader.h>

class BackgroundDownloader : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundDownloader(kiwix::Downloader* downloader);
    virtual ~BackgroundDownloader();

signals:
    void confirmStartDownload(const QString& bookID, const QString& did);

public slots:
    void startDownload(const QString& bookId, const QString& uri, const QString& downloadDir);

private slots:
    void cleanup();

private:
    std::unique_ptr<QThread> m_thread;
    QMutex m_mutex;
    kiwix::Downloader* mp_downloader;
};

#endif // BACKGROUNDDOWNLOADER_H
