#include "library.h"
#include "kiwixapp.h"

#include <kiwix/manager.h>
#include <kiwix/tools.h>

#include <QtDebug>
#include <QtConcurrent/QtConcurrentRun>


class LibraryManipulator: public kiwix::LibraryManipulator {
  public:
    LibraryManipulator(Library* p_library)
        : kiwix::LibraryManipulator(p_library->getKiwixLibrary())
        , mp_library(p_library)
    {}
    virtual ~LibraryManipulator() {}
    bool addBookToLibrary(kiwix::Book book) {
        auto ret = mp_library->mp_library->addBook(book);
        emit(mp_library->booksChanged());
        return ret;
    }
    void addBookmarkToLibrary(kiwix::Bookmark bookmark) {
        mp_library->mp_library->addBookmark(bookmark);
    }
    Library* mp_library;
};

Library::Library(const QString& libraryDirectory)
  : mp_library(std::make_shared<kiwix::Library>()),
    m_libraryDirectory(libraryDirectory)
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
        auto& book = mp_library->getBookByPath(zimPath.toStdString());
        return QString::fromStdString(book.getId());
    } catch(std::out_of_range& e) { }

    kiwix::Manager manager(mp_library);
    auto id =  manager.addBookFromPathAndGetId(zimPath.toStdString());
    if (id == "") {
        throw std::invalid_argument("invalid zim file");
    }
    save();
    emit(booksChanged());
    return QString::fromStdString(id);
}

std::shared_ptr<zim::Archive> Library::getArchive(const QString &zimId)
{
    return mp_library->getArchiveById(zimId.toStdString());
}

std::shared_ptr<zim::Searcher> Library::getSearcher(const QString &zimId)
{
    return mp_library->getSearcherById(zimId.toStdString());
}

QStringList Library::getBookIds() const
{
    QStringList list;
    for(auto& id: mp_library->getBooksIds()) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

QStringList Library::listBookIds(const kiwix::Filter& filter, kiwix::supportedListSortBy sortBy, bool ascending) const
{
    QStringList list;
    auto bookIds = mp_library->filter(filter);
    mp_library->sort(bookIds, sortBy, ascending);
    for(auto& id: bookIds) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

void Library::addBookToLibrary(kiwix::Book &book)
{
    mp_library->addBook(book);
}

void Library::removeBookFromLibraryById(const QString& id) {
    mp_library->removeBookById(id.toStdString());
}

void Library::addBookmark(kiwix::Bookmark &bookmark)
{
    mp_library->addBookmark(bookmark);
    emit bookmarksChanged();
}

void Library::removeBookmark(const QString &zimId, const QString &url)
{
    mp_library->removeBookmark(zimId.toStdString(), url.toStdString());
    emit bookmarksChanged();
}

void Library::save()
{
    mp_library->writeToFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(),"library.xml"));
    mp_library->writeBookmarksToFile(kiwix::appendToDirectory(m_libraryDirectory.toStdString(), "library.bookmarks.xml"));
}

void Library::setMonitorDirZims(QStringList zimList)
{
    m_monitorDirZims = zimList;
}

QStringList Library::getLibraryZimsFromDir(QString dir) const
{
    QStringList zimsInDir;
    for (auto str : getBookIds()) {
        auto filePath = QString::fromStdString(getBookById(str).getPath());
        QDir absoluteDir = QFileInfo(filePath).absoluteDir();
        if (absoluteDir == dir) {
            zimsInDir.push_back(filePath);
        }
    }
    return zimsInDir;
}

void Library::loadMonitorDir(QString monitorDir)
{
    QMutex mutex;
    QMutexLocker locker(&mutex);
    const QDir dir(monitorDir);
    QStringList newDirEntries = dir.entryList({"*.zim"});
    QStringList oldDirEntries = m_monitorDirZims;
    for (auto &str : newDirEntries) {
        str = QDir::toNativeSeparators(monitorDir + "/" + str);
    }
    QSet<QString> newDir, oldDir;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    newDir = QSet<QString>::fromList(newDirEntries);
    oldDir = QSet<QString>::fromList(oldDirEntries);
#else
    newDir = QSet<QString>(newDirEntries.begin(), newDirEntries.end());
    oldDir = QSet<QString>(oldDirEntries.begin(), oldDirEntries.end());
#endif
    QStringList addedZims = (newDir - oldDir).values();
    QStringList removedZims = (oldDir - newDir).values();
    auto manipulator = LibraryManipulator(this);
    auto manager = kiwix::Manager(&manipulator);
    bool needsRefresh = !removedZims.empty();
    for (auto book : addedZims) {
        needsRefresh |= manager.addBookFromPath(book.toStdString());
    }
    for (auto bookPath : removedZims) {
        removeBookFromLibraryById(QString::fromStdString(mp_library->getBookByPath(bookPath.toStdString()).getId()));
    }
    if (needsRefresh) {
        setMonitorDirZims(newDir.values());
        emit(booksChanged());
    }
}

void Library::asyncLoadMonitorDir(QString dir)
{
    QtConcurrent::run( [=]() {
        loadMonitorDir(dir);
    });
}

const kiwix::Book &Library::getBookById(QString id) const
{
    return mp_library->getBookById(id.toStdString());
}
