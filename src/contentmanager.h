#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include "library.h"
#include "contentmanagerview.h"
#include "opdsrequestmanager.h"
#include "contenttypefilter.h"
#include "contentmanagermodel.h"
#include "downloadmanagement.h"

class ContentManager : public DownloadManager
{
    Q_OBJECT
    Q_PROPERTY(bool isLocal MEMBER m_local READ isLocal WRITE setLocal NOTIFY localChanged)

public: // types
    typedef QList<QPair<QString, QString>> LanguageList;
    typedef QList<QPair<QString, QString>> FilterList;
    typedef ContentManagerModel::BookInfo     BookInfo;
    typedef ContentManagerModel::BookInfoList BookInfoList;
    typedef Library::QStringSet QStringSet;

    enum class BookState
    {
        // Nothing known about a book with that id
        INVALID,

        // Only (some) metadata is available for the book, however neither a
        // ZIM-file nor a URL is associated with it.
        METADATA_ONLY,

        // No ZIM file is associated with the book but a URL is provided.
        AVAILABLE_ONLINE,

        // The book is being downloaded.
        DOWNLOADING,

        // The book started downloading, but the download is currently paused.
        DOWNLOAD_PAUSED,

        // The book started downloading but the download was stopped due to
        // errors.
        DOWNLOAD_ERROR,

        // A valid ZIM file path is associated with the book and no evidence
        // about any issues with that ZIM file has so far been obtained.
        AVAILABLE_LOCALLY_AND_HEALTHY,

        // A ZIM file path is associated with the book but no such file seems
        // to exist (may be caused by missing read permissions for the directory
        // containing the ZIM file).
        ERROR_MISSING_ZIM_FILE,

        // A ZIM file is associated with the book but it cannot be opened
        // due to issues with its content.
        ERROR_CORRUPTED_ZIM_FILE
    };


public: // functions
    ContentManager(Library* library);

    ContentManagerView* getView() { return mp_view; }
    void setLocal(bool local);
    void setCurrentLanguage(FilterList languageList);
    void setCurrentCategoryFilter(QStringList categoryList);
    void setCurrentContentTypeFilter(FilterList contentTypeFilter);
    bool isLocal() const { return m_local; }
    QStringList getCategories() const { return m_categories; }
    LanguageList getLanguages() const { return m_languages; }

    void setMonitoredDirectories(QStringSet dirList);

signals:
    void filterParamsChanged();
    void booksChanged();
    void oneBookChanged(const QString&);
    void bookRemoved(const QString&);
    void downloadsChanged();
    void currentLangChanged();
    void pendingRequest(const bool);
    void categoriesLoaded(QStringList);
    void languagesLoaded(LanguageList);
    void localChanged(const bool);

public slots:
    QStringList getTranslations(const QStringList &keys);
    BookInfo getBookInfos(QString id, const QStringList &keys);
    BookState getBookState(QString id);
    void openBook(const QString& id);
    void openBookPreview(const QString& id);
    void downloadBook(const QString& id);
    void updateLibrary();
    void setSearch(const QString& search);
    void setSortBy(const QString& sortBy, const bool sortOrderAsc);
    // eraseBook() asks for confirmation (reallyEraseBook() doesn't)
    void eraseBook(const QString& id);
    void updateRemoteLibrary(const QString& content);
    void updateLanguages(const QString& content);
    void updateCategories(const QString& content);
    void pauseBook(const QString& id, QModelIndex index);
    void resumeBook(const QString& id, QModelIndex index);
    void cancelBook(const QString& id);
    void onCustomContextMenu(const QPoint &point);
    void openBookWithIndex(const QModelIndex& index);
    void updateDownload(QString bookId, const DownloadInfo& downloadInfo);
    void downloadWasCancelled(const QString& id);
    void handleError(QString errSummary, QString errDetails);

private: // types
    struct MonitoredZimFileInfo
    {
        enum ZimFileStatus
        {
            // try to add this file to the library right away
            PROCESS_NOW,

            // the file is known to be downloaded by our own download manager
            BEING_DOWNLOADED_BY_US,

            // the file was added to the library successfully
            ADDED_TO_THE_LIBRARY,

            // the attempt to add the file to the library failed
            COULD_NOT_BE_ADDED_TO_THE_LIBRARY,

            // the file couldn't be added to the library earlier and hasn't
            // changed since then
            UNCHANGED_KNOWN_BAD_ZIM_FILE,

            // try to add this file to the library at a later time
            PROCESS_LATER,

            // this file is known to be enqueued for later processing
            DEFERRED_PROCESSING_ALREADY_PENDING
        };

        bool fileKeepsBeingModified() const;
        void updateStatus(const MonitoredZimFileInfo& prevInfo);

        ZimFileStatus status = PROCESS_NOW;
        QDateTime lastModified;
    };

    typedef QMap<QString, MonitoredZimFileInfo> ZimFileName2InfoMap;

private: // functions
    QStringList getBookIds();
    // reallyEraseBook() doesn't ask for confirmation (unlike eraseBook())
    void reallyEraseBook(const QString& id, bool moveToTrash);
    void eraseBookFilesFromComputer(const std::string& bookPath, bool moveToTrash);
    void updateModel();
    void setCategories();
    void setLanguages();
    QStringSet getLibraryZims(QString dirPath) const;
    void asyncUpdateLibraryFromDir(QString dir);
    void updateLibraryFromDir(QString dir);
    void handleDisappearedZimFiles(const QString& dirPath, const QStringSet& fileNames);
    size_t handleNewZimFiles(const QString& dirPath, const QStringSet& fileNames);
    bool handleZimFileInMonitoredDirLogged(QString dirPath, QString fileName);
    int handleZimFileInMonitoredDir(QString dirPath, QString fileName);
    MonitoredZimFileInfo getMonitoredZimFileInfo(QString dir, QString fileName) const;
    void deferHandlingOfZimFileInMonitoredDir(QString dir, QString fileName);
    void handleZimFileInMonitoredDirDeferred(QString dirPath, QString fileName);
    bool handleDisappearedBook(QString bookId);

    // Get the book with the specified id from
    // the remote or local library (in that order).
    const kiwix::Book& getRemoteOrLocalBook(const QString &id);
    QString getRemoteLibraryUrl() const;

    void startDownload(QString bookId) override;
    void removeDownload(QString bookId);
    void downloadDisappeared(QString bookId);
    void downloadCompleted(QString bookId, QString path);

private: // data
    Library* mp_library;
    kiwix::LibraryPtr mp_remoteLibrary;
    OpdsRequestManager m_remoteLibraryManager;
    ContentManagerView* mp_view;
    bool m_local = true;
    QString m_currentLanguage;
    QString m_searchQuery;
    QString m_categoryFilter = "all";
    QStringList m_contentTypeFilters;
    kiwix::supportedListSortBy m_sortBy = kiwix::UNSORTED;
    bool m_sortOrderAsc = true;
    LanguageList m_languages;
    QStringList m_categories;

    ContentManagerModel *managerModel;
    QMutex remoteLibraryLocker;

    QFileSystemWatcher m_watcher;
    QMutex m_updateFromDirMutex;
    QMap<QString, ZimFileName2InfoMap> m_knownZimsInDir;
};

#endif // CONTENTMANAGER_H
