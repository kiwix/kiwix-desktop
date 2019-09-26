#include "library.h"
#include "kiwixapp.h"

#include <kiwix/manager.h>
#include "kiwixapp.h"

#include <QtDebug>

class LibraryManipulator: public kiwix::LibraryManipulator {
  public:
    LibraryManipulator(Library* p_library)
        : mp_library(p_library) {}
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

Library::Library()
{
    auto manipulator = LibraryManipulator(this);
    auto manager = kiwix::Manager(&manipulator);
    m_libraryDirectory = KiwixApp::instance()->getLibraryDirectory();
    qInfo() << "Library directory :" << m_libraryDirectory;
    manager.readFile(appendToDirectory(m_libraryDirectory.toStdString(),"library.xml"), false);
    manager.readBookmarkFile(appendToDirectory(m_libraryDirectory.toStdString(),"library.bookmarks.xml"));
    emit(booksChanged());
}

Library::~Library()
{
    save();
}

QString Library::openBookFromPath(const QString &zimPath)
{
    for(auto it=m_readersMap.begin();
        it != m_readersMap.end();
        it++)
    {
        if(QString::fromStdString(it.value()->getZimFilePath()) == zimPath)
            return it.key();
    }
    qInfo() << "Opening" << zimPath;
    auto zimPath_ = zimPath.toStdString();
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(zimPath_));
    auto id = QString::fromStdString(reader->getId());
    kiwix::Book b;
    b.update(*reader);
    m_library.addBook(b);
    m_readersMap[id] = reader;
    save();
    emit(booksChanged());
    return id;
}

QString Library::openBookById(const QString& id)
{
    auto& b = m_library.getBookById(id.toStdString());
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(b.getPath()));
    m_readersMap[id] = reader;
    return id;
}

std::shared_ptr<kiwix::Reader> Library::getReader(const QString &zimId)
{
    auto it = m_readersMap.find(zimId);
    if (it != m_readersMap.end())
        return it.value();
    // No reader, try to open the file
    try {
        openBookById(zimId);
        return m_readersMap.find(zimId).value();
    } catch(...) {}
    return nullptr;
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
    m_library.writeToFile(appendToDirectory(m_libraryDirectory.toStdString(),"library.xml"));
    m_library.writeBookmarksToFile(appendToDirectory(m_libraryDirectory.toStdString(), "library.bookmarks.xml"));
}

kiwix::Book &Library::getBookById(QString id)
{
    return m_library.getBookById(id.toStdString());
}
