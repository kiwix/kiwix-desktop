#ifndef DOWNLOADMANAGEMENT_H
#define DOWNLOADMANAGEMENT_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QString>
#include <QVariant>
#include <QWaitCondition>

#include <chrono>
#include <memory>

#include <kiwix/downloader.h>

#include "library.h"

typedef QMap<QString, QVariant> DownloadInfo;

template<class T>
class ThreadSafeQueue
{
public:
    void enqueue(const T& x)
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        m_queue.enqueue(x);
        m_queueIsNotEmpty.wakeAll();
    }

    T dequeue()
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        if ( m_queue.isEmpty() )
            m_queueIsNotEmpty.wait(&m_mutex);

        return m_queue.dequeue();
    }

    bool isEmpty() const
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        return m_queue.isEmpty();
    }

private: // data
    mutable QMutex  m_mutex;
    QQueue<T>       m_queue;
    QWaitCondition  m_queueIsNotEmpty;
};

class DownloadState
{
public: // types
    enum Status {
        UNKNOWN,
        WAITING,
        DOWNLOAD_ERROR,
        DOWNLOADING,
        PAUSED
    };


public: // data

    double progress = 0;
    QString completedLength;
    Status status = UNKNOWN;

public: // functions
    void update(const DownloadInfo& info);
    QString getDownloadSpeed() const;

    // time in seconds since last update
    double timeSinceLastUpdate() const;

private: // data
    QString downloadSpeed;
    std::chrono::steady_clock::time_point lastUpdated;
};

class DownloadManager : public QObject
{
    Q_OBJECT

public: // types
    typedef std::shared_ptr<DownloadState> DownloadStatePtr;

private:
    // BookId -> DownloadState map
    class Downloads
    {
    private:
        typedef QMap<QString, DownloadStatePtr> ImplType;

    public:
        void set(const QString& id, DownloadStatePtr d) {
            const QMutexLocker threadSafetyGuarantee(&mutex);
            impl[id] = d;
        }

        DownloadStatePtr value(const QString& id) const {
            const QMutexLocker threadSafetyGuarantee(&mutex);
            return impl.value(id);
        }

        QList<QString> keys() const {
            const QMutexLocker threadSafetyGuarantee(&mutex);
            return impl.keys();
        }

        void remove(const QString& id) {
            const QMutexLocker threadSafetyGuarantee(&mutex);
            impl.remove(id);
        }

    private:
        ImplType impl;
        mutable QMutex mutex;
    };

public: // functions
    DownloadManager(const Library* lib, kiwix::Downloader *downloader);
    virtual ~DownloadManager();

    bool downloadingFunctionalityAvailable() const;

    void startDownloadUpdaterThread();

    DownloadInfo getDownloadInfo(QString bookId) const;
    void restoreDownloads();

    // returns the download id
    std::string startDownload(const kiwix::Book& book, const QString& downloadDirPath);
    void pauseDownload(const QString& bookId);
    void resumeDownload(const QString& bookId);
    bool cancelDownload(const QString& bookId);
    void removeDownload(QString bookId);

    DownloadStatePtr getDownloadState(QString bookId) const
    {
        return m_downloads.value(bookId);
    }

signals:
    void downloadUpdated(QString bookId, const DownloadInfo& );
    void downloadDisappeared(QString bookId);

private: // types
    typedef ThreadSafeQueue<QString> RequestQueue;

private: // functions
    void processDownloadActions();
    void updateDownload(QString bookId);

private: // data
    const Library* const     mp_library;
    kiwix::Downloader* const mp_downloader;
    Downloads                m_downloads;
    QThread*                 mp_downloadUpdaterThread = nullptr;
    RequestQueue             m_requestQueue;
};

#endif // DOWNLOADMANAGEMENT_H
