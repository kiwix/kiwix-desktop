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

public:

    explicit ContentManager(Library* library, kiwix::Downloader *downloader, QObject *parent = nullptr);
    virtual ~ContentManager() {}

    ContentManagerView* getView() { return mp_view; }
    QStringList getDownloadIds();
private:
    Library* mp_library;
    kiwix::Downloader* mp_downloader;
    ContentManagerView* mp_view;
    int m_booksPerPage = 10;
    int m_currentPage = 0;
    void setCurrentPage(int currentPage) {
        m_currentPage = max(0, min(currentPage, getNbPages()));
        emit(booksChanged());
    }

    QStringList getBookIds() {
        return mp_library->getBookIds().mid(getStartBookIndex(), m_booksPerPage);
    }


signals:
    void booksChanged();
    void downloadsChanged();

public slots:
    int getNbPages() {
        return round(float(mp_library->getBookIds().length()) / m_booksPerPage);
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
};

#endif // CONTENTMANAGER_H
