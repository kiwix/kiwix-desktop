#include "library.h"


#include <kiwix/manager.h>

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
    qInfo() << QString::fromStdString(getDataDirectory());
    manager.readFile(appendToDirectory(getDataDirectory(),"library.xml"), false);
    manager.readBookmarkFile(appendToDirectory(getDataDirectory(),"library.bookmarks.xml"));
    qInfo() << getBookIds().length();
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
    auto _id(reader->getId());
    auto id = QString::fromStdString(_id + ".zim");
    kiwix::Book b;
    b.update(*reader);
    m_library.addBook(b);
    m_readersMap[id] = reader;
    save();
    emit(booksChanged());
    return id;
}

QString Library::openBookById(const QString& _id)
{
    auto& b = m_library.getBookById(_id.toStdString());
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(b.getPath()));
    auto id = _id + ".zim";
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
        QString _id = zimId;
        if (_id.endsWith(".zim")) _id.resize(_id.size()-4);
        openBookById(_id);
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

QStringList Library::listBookIds(const QString &query)
{
    QStringList list;
    for(auto& id: m_library.listBooksIds(kiwix::VALID|kiwix::LOCAL,
                                         kiwix::UNSORTED,
                                         query.toStdString())) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

void Library::addBookToLibrary(kiwix::Book &book)
{
    m_library.addBook(book);
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
    m_library.writeToFile(appendToDirectory(getDataDirectory(),"library.xml"));
    m_library.writeBookmarksToFile(appendToDirectory(getDataDirectory(), "library.bookmarks.xml"));
}

kiwix::Book &Library::getBookById(QString id)
{
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
    }
    return m_library.getBookById(id.toStdString());
}
