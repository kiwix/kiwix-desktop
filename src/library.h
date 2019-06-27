#ifndef LIBRARY_H
#define LIBRARY_H

#include <kiwix/book.h>
#include <kiwix/library.h>
#include <kiwix/reader.h>
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
    Library();
    virtual ~Library();
    QString openBookFromPath(const QString& zimPath);
    std::shared_ptr<kiwix::Reader> getReader(const QString& zimId);
    QStringList getBookIds();
    QStringList listBookIds(const kiwix::Filter& filter);
    const std::vector<kiwix::Bookmark>& getBookmarks() { return m_library.getBookmarks(); }
    void addBookToLibrary(kiwix::Book& book);
    void removeBookFromLibraryById(const QString& id);
    void addBookmark(kiwix::Bookmark& bookmark);
    void removeBookmark(const QString& zimId, const QString& url);
    void save();
public slots:
    QString openBookById(const QString& _id);
    kiwix::Book& getBookById(QString id);

signals:
    void booksChanged();
    void bookmarksChanged();

private:
    kiwix::Library m_library;
    QMap<QString, std::shared_ptr<kiwix::Reader>> m_readersMap;
    QString m_libraryDirectory;
friend class LibraryManipulator;
};

#endif // LIBRARY_H
