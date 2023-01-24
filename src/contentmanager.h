#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <QMap>
#include <math.h>
#include "library.h"
#include "contentmanagerview.h"
#include <kiwix/downloader.h>
#include "opdsrequestmanager.h"
#include "contenttypefilter.h"
#include "backgrounddownloader.h"

class ContentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bookIds READ getBookIds NOTIFY booksChanged)
    Q_PROPERTY(QStringList downloadIds READ getDownloadIds NOTIFY downloadsChanged)
    Q_PROPERTY(QString currentLanguage MEMBER m_currentLanguage WRITE setCurrentLanguage NOTIFY currentLangChanged)

public:
    explicit ContentManager(Library* library, kiwix::Downloader *downloader, QObject *parent = nullptr);
    virtual ~ContentManager() {}

    ContentManagerView* getView() { return mp_view; }
    void setLocal(bool local);
    QStringList getDownloadIds();
    void setCurrentLanguage(QString language);
    void setCurrentCategoryFilter(QString category);
    void setCurrentContentTypeFilter(QList<ContentTypeFilter*>& contentTypeFilter);

private:
    Library* mp_library;
    kiwix::Library m_remoteLibrary;
    kiwix::Downloader* mp_downloader;
    BackgroundDownloader* mp_background_downloader;
    OpdsRequestManager m_remoteLibraryManager;
    ContentManagerView* mp_view;
    bool m_local = true;
    QString m_currentLanguage;
    QString m_searchQuery;
    QString m_categoryFilter = "all";
    QList<ContentTypeFilter*> m_contentTypeFilters;
    kiwix::supportedListSortBy m_sortBy = kiwix::UNSORTED;
    bool m_sortOrderAsc = true;

    QStringList getBookIds();
    void eraseBookFilesFromComputer(const QString dirPath, const QString filename);

signals:
    void filterParamsChanged();
    void booksChanged();
    void oneBookChanged(const QString&);
    void bookRemoved(const QString&);
    void downloadsChanged();
    void currentLangChanged();
    void pendingRequest(const bool);

    void backgroundStartDownload(const QString&, const QString&, const QString&);
    void backgroundCompleteDownload(const QString&);
    void backgroundPauseDownload(const QString&);
    void backgroundResumeDownload(const QString&);
    void backgroundCancelDownload(const QString&, const QString&);

public slots:
    QStringList getTranslations(const QStringList &keys);
    QStringList getBookInfos(QString id, const QStringList &keys);
    void openBook(const QString& id);
    QStringList updateDownloadInfos(QString id, const QStringList& keys);
    QString startDownloadBook(const QString& id);
    QString downloadBook(const QString& bookId, const QString& did);
    void updateLibrary();
    void setSearch(const QString& search);
    void setSortBy(const QString& sortBy, const bool sortOrderAsc);
    void eraseBook(const QString& id);
    void updateRemoteLibrary(const QString& content);
    void pauseBook(const QString& id);
    void resumeBook(const QString& id);
    void startCancelBook(const QString& id);
    void completeCancelBook(const QString& bookId, const QString& path);
};

#endif // CONTENTMANAGER_H
