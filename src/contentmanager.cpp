#include "contentmanager.h"

#include "kiwixapp.h"
#include <kiwix/tools/networkTools.h>
#include <kiwix/tools/otherTools.h>
#include <kiwix/manager.h>

#include <QDebug>
#include <QUrlQuery>
#include <QUrl>
#include <QDir>
#include <QStorageInfo>

ContentManager::ContentManager(Library* library, kiwix::Downloader* downloader, QObject *parent)
    : QObject(parent),
      mp_library(library),
      mp_downloader(downloader),
      m_remoteLibraryManager()
{
    // mp_view will be passed to the tab who will take ownership,
    // so, we don't need to delete it.
    mp_view = new ContentManagerView();
    mp_view->registerObject("contentManager", this);
    mp_view->setHtml();
    setCurrentLanguage(QLocale().name().split("_").at(0));
    connect(mp_library, &Library::booksChanged, this, [=]() {emit(this->booksChanged());});
    connect(this, &ContentManager::filterParamsChanged, this, &ContentManager::updateLibrary);
    connect(&m_remoteLibraryManager, &OpdsRequestManager::requestReceived, this, &ContentManager::updateRemoteLibrary);
}

void ContentManager::setLocal(bool local) {
    if (local == m_local) {
        return;
    }
    m_local = local;
    emit(filterParamsChanged());
}

#define ADD_V(KEY, METH) {if(key==KEY) values.append(QString::fromStdString((b->METH())));}
QStringList ContentManager::getBookInfos(QString id, const QStringList &keys)
{
    QStringList values;
    kiwix::Book* b = [=]()->kiwix::Book* {
        try {
            return &mp_library->getBookById(id);
        } catch (...) {
            try {
                return &m_remoteLibrary.getBookById(id.toStdString());
            } catch(...) { return nullptr; }
        }
    }();

    if (nullptr == b){
        for(auto& key:keys) {
            values.append("");
        }
        return values;
    }

    for(auto& key: keys){
        ADD_V("id", getId);
        ADD_V("path", getPath);
        ADD_V("title", getTitle);
        ADD_V("description", getDescription);
        ADD_V("language", getLanguage);
        ADD_V("creator", getCreator);
        ADD_V("publisher", getPublisher);
        ADD_V("date", getDate);
        ADD_V("url", getUrl);
        ADD_V("name", getName);
        ADD_V("origId", getOrigId);
        ADD_V("faviconMimeType", getFaviconMimeType);
        ADD_V("downloadId", getDownloadId);
        ADD_V("faviconUrl", getFaviconUrl);
        if (key == "favicon") {
            auto s = b->getFavicon();
            values.append(QByteArray::fromStdString(s).toBase64());
        }
        if (key == "size") {
            values.append(QString::number(b->getSize()));
        }
        if (key == "articleCount") {
            values.append(QString::number(b->getArticleCount()));
        }
        if (key == "mediaCount") {
            values.append(QString::number(b->getMediaCount()));
        }
        if (key == "tags") {
            QStringList tagList = QString::fromStdString(b->getTags()).split(';');
            tagList = tagList.filter(QRegExp("^(?!_).*"));
            QString s = tagList.join(" ");
            values.append(s);
        }
    }
    return values;
}
#undef ADD_V

void ContentManager::openBook(const QString &id)
{
    QUrl url("zim://"+id+".zim/");
    KiwixApp::instance()->openUrl(url, true);
}

#define ADD_V(KEY, METH) {if(key==KEY) {values.append(QString::fromStdString((d->METH()))); continue;}}
QStringList ContentManager::updateDownloadInfos(QString id, const QStringList &keys)
{
    QStringList values;
    if (!mp_downloader) {
        for(auto& key: keys)
            values.append("");
        return values;
    }
    auto& b = mp_library->getBookById(id);
    kiwix::Download* d;
    try {
        d = mp_downloader->getDownload(b.getDownloadId());
    } catch(...) {
        b.setDownloadId("");
        mp_library->save();
        emit(mp_library->booksChanged());
        return values;
    }

    d->updateStatus(true);
    if (d->getStatus() == kiwix::Download::K_COMPLETE) {
        QString tmp(QString::fromStdString(d->getPath()));
        b.setPath(QDir::toNativeSeparators(tmp).toStdString());
        b.setDownloadId("");
        b.setPathValid(true);
        mp_library->save();
        mp_library->bookmarksChanged();
        if (!m_local) {
            emit(oneBookChanged(id));
        } else {
            emit(mp_library->booksChanged());
        }
    }
    for(auto& key: keys){
        ADD_V("id", getDid);
        if(key == "status") {
            switch(d->getStatus()){
            case kiwix::Download::K_ACTIVE:
                values.append("active");
                break;
            case kiwix::Download::K_WAITING:
                values.append("waiting");
                break;
            case kiwix::Download::K_PAUSED:
                values.append("paused");
                break;
            case kiwix::Download::K_ERROR:
                values.append("error");
                break;
            case kiwix::Download::K_COMPLETE:
                values.append("completed");
                break;
            case kiwix::Download::K_REMOVED:
                values.append("removed");
                break;
            default:
                values.append("unknown");
            }
            continue;
        }
        ADD_V("followedBy", getFollowedBy);
        ADD_V("path", getPath);
        if(key == "totalLength") {
            values.append(QString::number(d->getTotalLength()));
        }
        if(key == "completedLength") {
            values.append(QString::number(d->getCompletedLength()));
        }
        if(key == "downloadSpeed") {
            values.append(QString::number(d->getDownloadSpeed()));
        }
        if(key == "verifiedLength") {
            values.append(QString::number(d->getVerifiedLength()));
        }
    }
    return values;
}
#undef ADD_V

QString ContentManager::downloadBook(const QString &id)
{
    if (!mp_downloader)
        return "";
    auto& book = [&]()->kiwix::Book& {
        try {
            return m_remoteLibrary.getBookById(id.toStdString());
        } catch (...) {
            return mp_library->getBookById(id);
        }
    }();
    QStorageInfo storage(QString::fromStdString(getDataDirectory()));
    if (book.getSize() > storage.bytesAvailable()) {
        return "storage_error";
    }
    auto booksList = mp_library->getBookIds();
    for (auto b : booksList)
        if (b.toStdString() == book.getId())
            return "";
    kiwix::Download *download;
    try {
        download = mp_downloader->startDownload(book.getUrl());
    } catch (std::exception& e) {
        return "";
    }
    book.setDownloadId(download->getDid());
    mp_library->addBookToLibrary(book);
    mp_library->save();
    emit(oneBookChanged(id));
    return QString::fromStdString(download->getDid());
}

void ContentManager::eraseBookFilesFromComputer(const QString fileToRemove)
{
    QString dirName = KiwixApp::instance()->getLibraryDirectory();
    QDir dir(dirName, fileToRemove);
    for(const QString& filename: dir.entryList()) {
        dir.remove(filename);
    }
}

void ContentManager::eraseBook(const QString& id)
{
    auto tabBar = KiwixApp::instance()->getTabWidget();
    int i = 1;
    while (i < tabBar->count() - 1) {
        WebView* webView = tabBar->widget(i);
        if (webView->zimId() == id) {
            tabBar->closeTab(i);
        } else {
            i++;
        }
    }
    kiwix::Book book = mp_library->getBookById(id);
    QString fileToRemove = QString::fromUtf8(getLastPathElement(book.getPath()).c_str()) + "*";
    eraseBookFilesFromComputer(fileToRemove);
    mp_library->removeBookFromLibraryById(id);
    mp_library->save();
    emit mp_library->bookmarksChanged();
    if (m_local) {
        emit(bookRemoved(id));
    } else {
        emit(oneBookChanged(id));
    }
    KiwixApp::instance()->getSettingsManager()->deleteSettings(id);
}

void ContentManager::pauseBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() == kiwix::Download::K_ACTIVE)
        download->pauseDownload();
}

void ContentManager::resumeBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() == kiwix::Download::K_PAUSED)
        download->resumeDownload();
}

void ContentManager::cancelBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() != kiwix::Download::K_COMPLETE) {
        download->cancelDownload();
    }
    QString fileToRemove = QString::fromUtf8(getLastPathElement(download->getPath()).c_str()) + "*";
    eraseBookFilesFromComputer(fileToRemove);
    mp_library->removeBookFromLibraryById(id);
    mp_library->save();
    emit(oneBookChanged(id));
}

QStringList ContentManager::getDownloadIds()
{
    QStringList list;
    if (!mp_downloader)
        return list;
    for(auto& id: mp_downloader->getDownloadIds()) {
        qInfo() << QString::fromStdString(id);
        list.append(QString::fromStdString(id));
    }
    return list;
}

void ContentManager::setCurrentLanguage(QString language)
{
    if (language.length() == 2) {
      try {
        language = QString::fromStdString(
                     kiwix::converta2toa3(language.toStdString()));
      } catch (std::out_of_range&) {}
    }
    m_currentLanguage = language;
    emit(currentLangChanged());
    emit(filterParamsChanged());
}

void ContentManager::setCurrentCategoryFilter(QString category)
{
    m_categoryFilter = category.toLower();
    emit(filterParamsChanged());
}

void ContentManager::updateLibrary() {
    if (m_local) {
        emit(pendingRequest(false));
        emit(booksChanged());
        return;
    }
    try {
        emit(pendingRequest(true));
        m_remoteLibraryManager.doUpdate(m_currentLanguage, m_categoryFilter);
    } catch (runtime_error&) {}
}

#define CATALOG_URL "library.kiwix.org"
void ContentManager::updateRemoteLibrary(const QString& content) {
    m_remoteLibrary = kiwix::Library();
    kiwix::Manager manager(&m_remoteLibrary);
    manager.readOpds(content.toStdString(), CATALOG_URL);
    emit(this->booksChanged());
    emit(this->pendingRequest(false));
}

void ContentManager::setSearch(const QString &search)
{
    m_searchQuery = search;
    emit(booksChanged());
}

QStringList ContentManager::getBookIds()
{
    kiwix::Filter filter;
    std::vector<std::string> tags;
    if (m_categoryFilter != "all" && m_categoryFilter != "other") {
        tags.push_back(m_categoryFilter.toStdString());
        filter.acceptTags(tags);
    }
    if (m_categoryFilter == "other") {
        auto categoryList = KiwixApp::instance()->getMainWindow()->getSideContentManager()->getCategoryList();
        for (auto& category: categoryList) {
            if (category != "Other") {
                tags.push_back(category.toLower().toStdString());
            }
        }
        filter.rejectTags(tags);
    }
    filter.query(m_searchQuery.toStdString());

    if (m_local) {
        filter.local(true);
        filter.valid(true);
        return mp_library->listBookIds(filter, m_sortBy, m_sortOrderAsc);
    } else {
        filter.remote(true);
        auto bookIds = m_remoteLibrary.filter(filter);
        m_remoteLibrary.sort(bookIds, m_sortBy, m_sortOrderAsc);
        QStringList list;
        for(auto& bookId:bookIds) {
            list.append(QString::fromStdString(bookId));
        }
        return list;
    }
}

void ContentManager::setSortBy(const QString& sortBy, const bool sortOrderAsc)
{
    if (sortBy == "unsorted") {
        m_sortBy = kiwix::UNSORTED;
    } else if (sortBy == "title") {
        m_sortBy = kiwix::TITLE;
    } else if (sortBy == "size") {
        m_sortBy = kiwix::SIZE;
    } else if (sortBy == "date") {
        m_sortBy = kiwix::DATE;
    }
    m_sortOrderAsc = sortOrderAsc;
    emit(booksChanged());
}