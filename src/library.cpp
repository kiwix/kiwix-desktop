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
  : mp_library(kiwix::Library::create()),
    m_libraryDirectory(libraryDirectory)
{
    auto manager = kiwix::Manager(LibraryManipulator(this));
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
    const auto archive =  mp_library->getArchiveById(zimId.toStdString());
    if ( ! archive ) {
        throw std::out_of_range("ZIM file doesn't exist (or cannot be opened)");
    }
    return archive;
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

namespace
{

const char BEINGDOWNLOADEDSUFFIX[] = ".beingdownloadedbykiwix";

std::string pseudoPathOfAFileBeingDownloaded(const std::string& path)
{
    return path + BEINGDOWNLOADEDSUFFIX;
}

std::string dropSuffix(const std::string& str, const std::string& suffix)
{
    const size_t s = suffix.size();
    const size_t n = str.size();
    return n > s && str.substr(n - s, s) == suffix
         ? str.substr(0, n - s)
         : str;
}

} // unnamed namespace

std::string Library::getBookFilePath(const QString& bookId) const
{
    const auto& book = getBookById(bookId);
    const std::string bookPath = book.getPath();
    return book.getDownloadId().empty()
         ? bookPath
         : dropSuffix(bookPath, BEINGDOWNLOADEDSUFFIX);
}

void Library::addBookBeingDownloaded(const kiwix::Book& book, QString downloadDir)
{
    const QString downloadUrl = QString::fromStdString(book.getUrl());

    // XXX: This works if the URL is a direct link to a ZIM file
    // XXX: rather than to a torrent or a metalink file. In those cases
    // XXX: the file name of the download will be discovered after the
    // XXX: the metalink or torrent file is downloaded. Then the real
    // XXX: file name of a book must be set via updateBookBeingDownloaded();
    const QString fileName = downloadUrl.split('/').back();

    const QString path = QDir(downloadDir).absoluteFilePath(fileName);
    const std::string stlPath = QDir::toNativeSeparators(path).toStdString();

    kiwix::Book bookCopy(book);
    bookCopy.setPath(pseudoPathOfAFileBeingDownloaded(stlPath));
    addBookToLibrary(bookCopy);
}

void Library::updateBookBeingDownloaded(const QString& bookId, const QString& bookPath)
{
    if ( bookPath.isEmpty() )
        return;

    const kiwix::Book& book = getBookById(bookId);
    const auto bookPseudoPath = pseudoPathOfAFileBeingDownloaded(bookPath.toStdString());
    if ( bookPseudoPath != book.getPath() ) {
        kiwix::Book bookCopy(book);
        bookCopy.setPath(bookPseudoPath);
        mp_library->addOrUpdateBook(bookCopy);
        save();
    }
}

bool Library::isBeingDownloadedByUs(QString path) const
{
    const auto fakePath = pseudoPathOfAFileBeingDownloaded(path.toStdString());
    try {
        mp_library->getBookByPath(fakePath);
        return true;
    } catch ( const std::out_of_range& ) {
        return false;
    }
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

void Library::setMonitorDirZims(QString monitorDir, QStringSet zimList)
{
    m_knownZimsInDir[monitorDir] = zimList;
}

Library::QStringSet Library::getLibraryZimsFromDir(QString dir) const
{
    QStringSet zimsInDir;
    for (auto str : getBookIds()) {
        auto filePath = QString::fromStdString(getBookById(str).getPath());
        if ( filePath.endsWith(BEINGDOWNLOADEDSUFFIX) )
                continue;
        QDir absoluteDir = QFileInfo(filePath).absoluteDir();
        if (absoluteDir == dir) {
            zimsInDir.insert(filePath);
        }
    }
    return zimsInDir;
}

void Library::updateFromDir(QString monitorDir)
{
    QMutexLocker locker(&m_updateFromDirMutex);
    const QDir dir(monitorDir);
    const QStringSet oldDirEntries = m_knownZimsInDir[monitorDir];
    QStringSet newDirEntries;
    for (const auto &file : dir.entryList({"*.zim"})) {
        newDirEntries.insert(QDir::toNativeSeparators(monitorDir + "/" + file));
    }
    QStringList addedZims = (newDirEntries - oldDirEntries).values();
    QStringList removedZims = (oldDirEntries - newDirEntries).values();
    auto manager = kiwix::Manager(LibraryManipulator(this));
    bool needsRefresh = !removedZims.empty();
    for (auto bookPath : addedZims) {
        if ( isBeingDownloadedByUs(bookPath) ) {
            // qDebug() << "DBG: Library::updateFromDir(): "
            //          << bookPath
            //          << " ignored since it is being downloaded by us.";
        } else {
            needsRefresh |= manager.addBookFromPath(bookPath.toStdString());
        }
    }
    for (auto bookPath : removedZims) {
        try {
            removeBookFromLibraryById(QString::fromStdString(mp_library->getBookByPath(bookPath.toStdString()).getId()));
        } catch (...) {}
    }
    if (needsRefresh) {
        emit(booksChanged());
        setMonitorDirZims(monitorDir, newDirEntries);
    }
}

void Library::asyncUpdateFromDir(QString dir)
{
    (void) QtConcurrent::run([=]() {
        updateFromDir(dir);
    });
}

const kiwix::Book &Library::getBookById(QString id) const
{
    return mp_library->getBookById(id.toStdString());
}
