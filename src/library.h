#ifndef LIBRARY_H
#define LIBRARY_H

#include <kiwix/book.h>
#include <kiwix/library.h>
#include <zim/archive.h>
#include <zim/search.h>
#include <qstring.h>
#include <memory>

#include <QObject>
#include <QSharedPointer>
#include <QMap>

#define TQS(v) (QString::fromStdString(v))
#define FORWARD_GETTER(METH) QString METH() const { return TQS(mp_book->METH()); }

#undef FORWARD_GETTER
#undef TQS

class LibraryManipulator;

class Library : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bookIds READ getBookIds NOTIFY booksChanged)
public:
    Library(const QString& libraryDirectory);
    virtual ~Library();
    QString openBookFromPath(const QString& zimPath);
    std::shared_ptr<zim::Archive> getArchive(const QString& zimId);
    std::shared_ptr<zim::Searcher> getSearcher(const QString& zimId);
    QStringList getBookIds() const;
    QStringList listBookIds(const kiwix::Filter& filter, kiwix::supportedListSortBy sortBy, bool ascending) const;
    const std::vector<kiwix::Bookmark> getBookmarks(bool onlyValidBookmarks = false) const { return m_library.getBookmarks(onlyValidBookmarks); }
    QStringList getLibraryZimsFromDir(QString dir) const;
    void setMonitorDirZims(QStringList zimList);
    void addBookToLibrary(kiwix::Book& book);
    void removeBookFromLibraryById(const QString& id);
    void addBookmark(kiwix::Bookmark& bookmark);
    void removeBookmark(const QString& zimId, const QString& url);
    void save();
    void loadMonitorDir(QString dir);
    void asyncLoadMonitorDir(QString dir);
    kiwix::Library& getKiwixLibrary() { return m_library; }
public slots:
    const kiwix::Book& getBookById(QString id) const;

signals:
    void booksChanged();
    void bookmarksChanged();

private:
    kiwix::Library m_library;
    QString m_libraryDirectory;
    QStringList m_monitorDirZims;
friend class LibraryManipulator;
};

#endif // LIBRARY_H
