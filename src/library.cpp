#include "library.h"
#include "kiwixapp.h"

#include <kiwix/manager.h>
#include <kiwix/tools.h>

#include <QtDebug>

class LibraryManipulator: public kiwix::LibraryManipulator {
  public:
    LibraryManipulator(Library* p_library)
        : kiwix::LibraryManipulator(&p_library->getKiwixLibrary())
        , mp_library(p_library)
    {}
    virtual ~LibraryManipulator() {}
    bool addBookToLibrary(kiwix::Book book) {
        auto ret = mp_library->m_library.addBook(book);
        emit(mp_library->booksChanged());
        return ret;
    }
    void addBookmarkToLibrary(kiwix::Bookmark bookmark) {
        mp_library->m_library.addBookmark(bookmark);
    }
    Library* mp_library;
};

Library::Library(const QString& libraryDirectory)
  : m_libraryDirectory(libraryDirectory)
{
    auto manipulator = LibraryManipulator(this);
    auto manager = kiwix::Manager(&manipulator);
    manager.readFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(),"library.xml"), false);
    manager.readBookmarkFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(),"library.bookmarks.xml"));
    emit(booksChanged());
}

Library::~Library()
{
    save();
}

QString Library::openBookFromPath(const QString &zimPath)
{
    try {
        auto& book = m_library.getBookByPath(zimPath.toStdString());
        return QString::fromStdString(book.getId());
    } catch(std::out_of_range& e) { }

    kiwix::Manager manager(&m_library);
    auto id =  manager.addBookFromPathAndGetId(zimPath.toStdString());
    if (id == "") {
        throw std::invalid_argument("invalid zim file");
    }
    save();
    emit(booksChanged());
    return QString::fromStdString(id);
}

std::shared_ptr<kiwix::Reader> Library::getReader(const QString &zimId)
{
    try {
      return m_library.getReaderById(zimId.toStdString());
    } catch (std::out_of_range& e) {
      return nullptr;
    }
}

std::shared_ptr<kiwix::Searcher> Library::getSearcher(const QString &zimId)
{
    auto searcher = std::make_shared<kiwix::Searcher>();
    try {
        searcher->add_reader(m_library.getReaderById(zimId.toStdString()).get());
    } catch(std::out_of_range& e) {
        return nullptr;
    }
    return searcher;
}

QStringList Library::getBookIds()
{
    QStringList list;
    for(auto& id: m_library.getBooksIds()) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

QStringList Library::listBookIds(const kiwix::Filter& filter, kiwix::supportedListSortBy sortBy, bool ascending)
{
    QStringList list;
    auto bookIds = m_library.filter(filter);
    m_library.sort(bookIds, sortBy, ascending);
    for(auto& id: bookIds) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

void Library::addBookToLibrary(kiwix::Book &book)
{
    m_library.addBook(book);
}

void Library::removeBookFromLibraryById(const QString& id) {
    m_library.removeBookById(id.toStdString());
}

void Library::addBookmark(kiwix::Bookmark &bookmark)
{
    m_library.addBookmark(bookmark);
    emit bookmarksChanged();
}

void Library::removeBookmark(const QString &zimId, const QString &url)
{
    m_library.removeBookmark(zimId.toStdString(), url.toStdString());
    emit bookmarksChanged();
}

void Library::save()
{
    m_library.writeToFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(),"library.xml"));
    m_library.writeBookmarksToFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(), "library.bookmarks.xml"));
}

const kiwix::Book &Library::getBookById(QString id) const
{
    return m_library.getBookById(id.toStdString());
}
