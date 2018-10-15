#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <math.h>
#include "library.h"
#include "contentmanagerview.h"

class ContentManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int booksPerPage MEMBER m_booksPerPage NOTIFY pagingChanged)
    Q_PROPERTY(int nbPages READ getNbPages NOTIFY pagingChanged)
    Q_PROPERTY(int currentPage MEMBER m_currentPage WRITE setCurrentPage NOTIFY pagingChanged)
    Q_PROPERTY(int startBookIndex READ getStartBookIndex NOTIFY pagingChanged)
    Q_PROPERTY(int endBookIndex READ getEndBookIndex NOTIFY pagingChanged)

public:

    explicit ContentManager(Library* library, QObject *parent = nullptr);
    virtual ~ContentManager() {}

    ContentManagerView* getView() { return mp_view; }
private:
    Library* mp_library;
    ContentManagerView* mp_view;
    int m_booksPerPage = 10;
    int m_currentPage = 0;
    void setCurrentPage(int currentPage) {
        m_currentPage = max(0, min(currentPage, getNbPages()));
        emit(pagingChanged());
    }


signals:
    void pagingChanged();

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
};

#endif // CONTENTMANAGER_H
