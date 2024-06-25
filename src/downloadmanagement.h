#ifndef DOWNLOADMANAGEMENT_H
#define DOWNLOADMANAGEMENT_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QVariant>
#include <QWaitCondition>

#include <chrono>
#include <memory>
#include <queue>

#include <kiwix/downloader.h>

#include "library.h"

typedef QMap<QString, QVariant> DownloadInfo;

template<class T>
class ThreadSafePriorityQueue
{
public:
    void enqueue(const T& x)
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        m_queue.push(x);
        m_queueIsNotEmpty.wakeAll();
    }

    T dequeue()
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        if ( m_queue.empty() )
            m_queueIsNotEmpty.wait(&m_mutex);

        const T ret = m_queue.top();
        m_queue.pop();
        return ret;
    }

    bool isEmpty() const
    {
        const QMutexLocker threadSafetyGuarantee(&m_mutex);
        return m_queue.empty();
    }

private: // data
    mutable QMutex  m_mutex;
    std::priority_queue<T> m_queue;
    QWaitCondition  m_queueIsNotEmpty;
};

class DownloadState
{
public: // types
    enum Action {
        UPDATE,
        START,
        PAUSE,
        RESUME,
        CANCEL
    };

    enum Status {
        UNKNOWN,
        WAITING,
        DOWNLOAD_ERROR,
        DOWNLOADING,
        PAUSE_REQUESTED,
        PAUSED,
        RESUME_REQUESTED,
        CANCEL_REQUESTED
    };

public: // data

    double progress = 0;
    QString completedLength;

public: // functions
    void update(const DownloadInfo& info);
    QString getDownloadSpeed() const;
    Status getStatus() const { return status; }
    void changeState(Action action);
    bool stateChangeHasBeenRequested() const
    {
       return status == PAUSE_REQUESTED
           || status == RESUME_REQUESTED
           || status == CANCEL_REQUESTED;
    }
    bool isLateUpdateInfo(const DownloadInfo& info) const;

    // time in seconds since last update
    double timeSinceLastUpdate() const;

private: // data
    Status status = UNKNOWN;
    QString downloadSpeed;
    std::chrono::steady_clock::time_point lastUpdated;
};

class DownloadManager : public QObject
{
    Q_OBJECT

public: // types
    typedef std::shared_ptr<DownloadState> DownloadStatePtr;
    typedef DownloadState::Action Action;

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

    void addRequest(Action action, QString bookId);

    // Throws a KiwixAppError in case of any foreseeable problem preventing a
    // successful download
    void checkThatBookCanBeDownloaded(const kiwix::Book& book, const QString& downloadDirPath);

    // returns the download id
    std::string startDownload(const kiwix::Book& book, const QString& downloadDirPath);
    void removeDownload(QString bookId);

    DownloadStatePtr getDownloadState(QString bookId) const
    {
        return m_downloads.value(bookId);
    }

signals:
    void error(QString errSummary, QString errDetails);
    void downloadUpdated(QString bookId, const DownloadInfo& );
    void downloadCancelled(QString bookId);
    void downloadDisappeared(QString bookId);

private: // types
    struct Request
    {
        Action  action;
        QString bookId;

        bool operator<(const Request& other) const {
            return this->action < other.action;
        }
    };

    typedef ThreadSafePriorityQueue<Request> RequestQueue;

private: // functions
    void processDownloadActions();
    void pauseDownload(const QString& bookId);
    void resumeDownload(const QString& bookId);
    void updateDownload(QString bookId);
    void cancelDownload(const QString& bookId);

private: // data
    const Library* const     mp_library;
    kiwix::Downloader* const mp_downloader;
    Downloads                m_downloads;
    QThread*                 mp_downloadUpdaterThread = nullptr;
    RequestQueue             m_requestQueue;
};

#endif // DOWNLOADMANAGEMENT_H
