#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <math.h>
#include "library.h"
#include "contentmanagerview.h"
#include <kiwix/downloader.h>

class ContentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int booksPerPage MEMBER m_booksPerPage NOTIFY booksChanged)
    Q_PROPERTY(int nbPages READ getNbPages NOTIFY booksChanged)
    Q_PROPERTY(int currentPage MEMBER m_currentPage WRITE setCurrentPage NOTIFY booksChanged)
    Q_PROPERTY(int startBookIndex READ getStartBookIndex NOTIFY booksChanged)
    Q_PROPERTY(int endBookIndex READ getEndBookIndex NOTIFY booksChanged)
    Q_PROPERTY(QStringList bookIds READ getBookIds NOTIFY booksChanged)
    Q_PROPERTY(QStringList downloadIds READ getDownloadIds NOTIFY downloadsChanged)
    Q_PROPERTY(QString currentLanguage MEMBER m_currentLanguage WRITE setCurrentLanguage NOTIFY currentLangChanged)

public:

    explicit ContentManager(Library* library, kiwix::Downloader *downloader, QObject *parent = nullptr);
    virtual ~ContentManager() {}

    ContentManagerView* getView() { return mp_view; }
    void setLocal(bool local);
    QStringList getDownloadIds();
private:
    Library* mp_library;
    kiwix::Library m_remoteLibrary;
    kiwix::Downloader* mp_downloader;
    ContentManagerView* mp_view;
    int m_booksPerPage = 10;
    int m_currentPage = 0;
    int m_totalBooks = 0;
    bool m_local = true;
    QString m_currentLanguage;
    void setCurrentPage(int currentPage) {
        m_currentPage = max(0, min(currentPage, getNbPages()-1));
        emit(booksChanged());
    }
    void setCurrentLanguage(QString language);

    QStringList getBookIds();

signals:
    void remoteParamsChanged();
    void booksChanged();
    void downloadsChanged();
    void currentLangChanged();

public slots:
    int getNbPages() {
        auto nbBooks = m_local ? mp_library->getBookIds().length() : m_totalBooks;
        return round(float(nbBooks) / m_booksPerPage);
    }
    int getStartBookIndex() {
        return m_currentPage * m_booksPerPage;
    }
    int getEndBookIndex() {
        return min((m_currentPage+1) * m_booksPerPage, mp_library->getBookIds().length());
    }
    QStringList getBookInfos(QString id, const QStringList &keys);
    void openBook(const QString& id);
    QStringList updateDownloadInfos(QString id, const QStringList& keys);
    QString downloadBook(const QString& id);
    void updateRemoteLibrary();
};

#endif // CONTENTMANAGER_H
