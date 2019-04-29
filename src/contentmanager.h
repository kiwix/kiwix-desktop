#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <math.h>
#include "library.h"
#include "contentmanagerview.h"
#include <kiwix/downloader.h>
#include "opdsrequestmanager.h"

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


    QStringList getBookIds();
    void eraseBookFilesFromComputer(const QString fileSelection);

signals:
    void filterParamsChanged();
    void booksChanged();
    void downloadsChanged();
    void currentLangChanged();
    void pendingRequest(const bool);

public slots:
    QStringList getBookInfos(QString id, const QStringList &keys);
    void openBook(const QString& id);
    QStringList updateDownloadInfos(QString id, const QStringList& keys);
    QString downloadBook(const QString& id);
    void updateLibrary();
    void setSearch(const QString& search);
    void eraseBook(const QString& id);
    void updateRemoteLibrary(const QString& content);
    void pauseBook(const QString& id);
    void resumeBook(const QString& id);
    void cancelBook(const QString& id);
};

#endif // CONTENTMANAGER_H
