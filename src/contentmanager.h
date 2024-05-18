#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include "library.h"
#include "contentmanagerview.h"
#include <kiwix/downloader.h>
#include "opdsrequestmanager.h"
#include "contenttypefilter.h"
#include "contentmanagermodel.h"

class ContentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLocal MEMBER m_local READ isLocal WRITE setLocal NOTIFY localChanged)

public: // types
    typedef QList<QPair<QString, QString>> LanguageList;
    typedef QList<QPair<QString, QString>> FilterList;
    typedef ContentManagerModel::BookInfo     BookInfo;
    typedef ContentManagerModel::BookInfoList BookInfoList;

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
    explicit ContentManager(Library* library, kiwix::Downloader *downloader, QObject *parent = nullptr);
    virtual ~ContentManager();

    ContentManagerView* getView() { return mp_view; }
    void setLocal(bool local);
    void setCurrentLanguage(FilterList languageList);
    void setCurrentCategoryFilter(FilterList category);
    void setCurrentContentTypeFilter(FilterList contentTypeFilter);
    bool isLocal() const { return m_local; }
    QStringList getCategories() const { return m_categories; }
    LanguageList getLanguages() const { return m_languages; }

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
    void downloadUpdated(QString bookId, const DownloadInfo& );

public slots:
    QStringList getTranslations(const QStringList &keys);
    BookInfo getBookInfos(QString id, const QStringList &keys);
    BookState getBookState(QString id);
    void openBook(const QString& id);
    void downloadBook(const QString& id);
    void downloadBook(const QString& id, QModelIndex index);
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
    // cancelBook() asks for confirmation (reallyCancelBook() doesn't)
    void cancelBook(const QString& id);
    void onCustomContextMenu(const QPoint &point);
    void openBookWithIndex(const QModelIndex& index);
    void updateDownloads();
    void updateDownload(QString bookId, const DownloadInfo& downloadInfo);

private: // functions
    QStringList getBookIds();
    // reallyCancelBook() doesn't ask for confirmation (unlike cancelBook())
    void reallyCancelBook(const QString& id);
    // reallyEraseBook() doesn't ask for confirmation (unlike eraseBook())
    void reallyEraseBook(const QString& id, bool moveToTrash);
    void eraseBookFilesFromComputer(const std::string& bookPath, bool moveToTrash);
    void updateModel();
    void setCategories();
    void setLanguages();

    // Get the book with the specified id from
    // the remote or local library (in that order).
    const kiwix::Book& getRemoteOrLocalBook(const QString &id);
    QString getRemoteLibraryUrl() const;

    void startDownloadUpdaterThread();
    std::string startDownload(const kiwix::Book& book);
    void removeDownload(QString bookId);
    void downloadStarted(const kiwix::Book& book, const std::string& downloadId);
    void downloadDisappeared(QString bookId);
    void downloadCompleted(QString bookId, QString path);
    DownloadInfo getDownloadInfo(QString bookId) const;
    void restoreDownloads();

private: // data
    Library* mp_library;
    kiwix::LibraryPtr mp_remoteLibrary;
    kiwix::Downloader* mp_downloader;
    ContentManagerModel::Downloads m_downloads;
    QThread* mp_downloadUpdaterThread = nullptr;
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
};

#endif // CONTENTMANAGER_H
