#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <math.h>
#include "library.h"
#include "contentmanagerview.h"
#include <kiwix/downloader.h>
#include "opdsrequestmanager.h"
#include "contenttypefilter.h"
#include "contentmanagermodel.h"

class ContentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bookIds READ getBookIds NOTIFY booksChanged)
    Q_PROPERTY(QStringList downloadIds READ getDownloadIds NOTIFY downloadsChanged)
    Q_PROPERTY(QString currentLanguage MEMBER m_currentLanguage WRITE setCurrentLanguage NOTIFY currentLangChanged)
    typedef QList<QPair<QString, QString>> LanguageList;

public:
    explicit ContentManager(Library* library, kiwix::Downloader *downloader, QObject *parent = nullptr);
    virtual ~ContentManager() {}

    ContentManagerView* getView() { return mp_view; }
    void setLocal(bool local);
    QStringList getDownloadIds();
    void setCurrentLanguage(QString language);
    void setCurrentCategoryFilter(QString category);
    void setCurrentContentTypeFilter(QList<ContentTypeFilter*>& contentTypeFilter);
    bool isLocal() const { return m_local; }
    QStringList getCategories() const { return m_categories; }
    LanguageList getLanguages() const { return m_languages; }

private:
    Library* mp_library;
    kiwix::Library m_remoteLibrary;
    kiwix::Downloader* mp_downloader;
    OpdsRequestManager m_remoteLibraryManager;
    ContentManagerView* mp_view;
    bool m_local = true;
    QString m_currentLanguage;
    QString m_searchQuery;
    QString m_categoryFilter = "all";
    QList<ContentTypeFilter*> m_contentTypeFilters;
    kiwix::supportedListSortBy m_sortBy = kiwix::UNSORTED;
    bool m_sortOrderAsc = true;
    LanguageList m_languages;
    QStringList m_categories;

    QStringList getBookIds();
    void eraseBookFilesFromComputer(const QString dirPath, const QString filename);
    QList<QMap<QString, QVariant>> getBooksList();
    ContentManagerModel *managerModel;
    QMutex remoteLibraryLocker;
    void setCategories();
    void setLanguages();

signals:
    void filterParamsChanged();
    void booksChanged();
    void oneBookChanged(const QString&);
    void bookRemoved(const QString&);
    void downloadsChanged();
    void currentLangChanged();
    void pendingRequest(const bool);
    void categoriesLoaded(QStringList);

public slots:
    QStringList getTranslations(const QStringList &keys);
    QMap<QString, QVariant> getBookInfos(QString id, const QStringList &keys);
    void openBook(const QString& id);
    QMap<QString, QVariant> updateDownloadInfos(QString id, const QStringList& keys);
    QString downloadBook(const QString& id);
    QString downloadBook(const QString& id, QModelIndex index);
    void updateLibrary();
    void setSearch(const QString& search);
    void setSortBy(const QString& sortBy, const bool sortOrderAsc);
    void eraseBook(const QString& id);
    void updateRemoteLibrary(const QString& content);
    void updateLanguages(const QString& content);
    void updateCategories(const QString& content);
    void pauseBook(const QString& id);
    void resumeBook(const QString& id);
    void cancelBook(const QString& id);
    void pauseBook(const QString& id, QModelIndex index);
    void resumeBook(const QString& id, QModelIndex index);
    void cancelBook(const QString& id, QModelIndex index);
    void onCustomContextMenu(const QPoint &point);
    void openBookWithIndex(const QModelIndex& index);
};

#endif // CONTENTMANAGER_H
