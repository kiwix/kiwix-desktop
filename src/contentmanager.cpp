#include "contentmanager.h"

#include "backgrounddownloader.h"
#include "kiwixapp.h"
#include "static_content.h"
#include <kiwix/manager.h>
#include <kiwix/tools.h>

#include <QDebug>
#include <QUrlQuery>
#include <QUrl>
#include <QDir>
#include <QStorageInfo>
#include <QMessageBox>

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

    // setup background downloader
    mp_background_downloader = new BackgroundDownloader(mp_downloader);
    connect(this, &ContentManager::backgroundStartDownload, mp_background_downloader, &BackgroundDownloader::startDownload);
    connect(mp_background_downloader, &BackgroundDownloader::confirmStartDownload, this, &ContentManager::downloadBook);

    connect(this, &ContentManager::backgroundCompleteDownload, mp_background_downloader, &BackgroundDownloader::completeDownload);
    connect(this, &ContentManager::backgroundPauseDownload, mp_background_downloader, &BackgroundDownloader::pauseDownload);
    connect(this, &ContentManager::backgroundResumeDownload, mp_background_downloader, &BackgroundDownloader::resumeDownload);
    connect(this, &ContentManager::backgroundCancelDownload, mp_background_downloader, &BackgroundDownloader::cancelDownload);
    connect(mp_background_downloader, &BackgroundDownloader::confirmCancelDownload, this, &ContentManager::completeCancelBook);
}

void ContentManager::setLocal(bool local) {
    if (local == m_local) {
        return;
    }
    m_local = local;
    emit(filterParamsChanged());
}

QStringList ContentManager::getTranslations(const QStringList &keys)
{
    QStringList translations;

    for(auto& key: keys) {
        translations.append(KiwixApp::instance()->getText(key));
    }
    return translations;
}

#define ADD_V(KEY, METH) {if(key==KEY) values.append(QString::fromStdString((b->METH())));}
QStringList ContentManager::getBookInfos(QString id, const QStringList &keys)
{
    QStringList values;
    const kiwix::Book* b = [=]()->const kiwix::Book* {
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
            (void) key;
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
        ADD_V("downloadId", getDownloadId);
        if (key == "favicon") {
            try {
                auto s = b->getIllustration(48)->getData();
                values.append(QByteArray::fromStdString(s).toBase64());
            } catch(...) {
                values.append(QByteArray());
            }
        }
        if (key == "faviconMimeType") {
            std::string mimeType;
            try {
                auto item = b->getIllustration(48);
                mimeType = item->mimeType;
            } catch (...) {
                const kiwix::Book::Illustration tempIllustration;
                mimeType = tempIllustration.mimeType;
            }
            values.append(QString::fromStdString(mimeType));
        }
        if (key == "faviconUrl") {
            std::string url;
            try {
                auto item = b->getIllustration(48);
                url = item->url;
            } catch (...) {
                const kiwix::Book::Illustration tempIllustration;
                url = tempIllustration.url;
            }
            values.append(QString::fromStdString(url));
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
            QMap<QString, bool> displayTagMap;
            for(auto tag: tagList) {
              if (tag[0] == "_") {
                auto splitTag = tag.split(":");
                displayTagMap[splitTag[0]] = splitTag[1] == "yes" ? true:false;
              }
            }
            QStringList displayTagList;
            if (displayTagMap["_videos"]) displayTagList << tr("Videos");
            if (displayTagMap["_pictures"]) displayTagList << tr("Pictures");
            if (!displayTagMap["_details"]) displayTagList << tr("Introduction only");
            QString s = displayTagList.join(", ");
            values.append(s);
        }
    }
    return values;
}
#undef ADD_V

void ContentManager::openBook(const QString &id)
{
    QUrl url("zim://"+id+".zim/");
    try {
        KiwixApp::instance()->openUrl(url, true);
    } catch (const std::exception& e) {
        auto tabBar = KiwixApp::instance()->getTabWidget();
        tabBar->closeTab(1);
        auto text = gt("zim-open-fail-text");
        text = text.replace("{{ZIM}}", QString::fromStdString(mp_library->getBookById(id).getPath()));
        auto title = gt("zim-open-fail-title");
        KiwixApp::instance()->showMessage(text, title, QMessageBox::Warning);
        mp_library->removeBookFromLibraryById(id);
        tabBar->setCurrentIndex(0);
        emit(booksChanged());
        return;
    }
}

QStringList ContentManager::updateDownloadInfos(QString id, const QStringList &keys)
{
    auto& b = mp_library->getBookById(id);

    /*
    kiwix::Download* d;
    try {
        d = mp_downloader->getDownload(b.getDownloadId());
    } catch(...) {
        kiwix::Book bCopy(b);
        bCopy.setDownloadId("");
        mp_library->getKiwixLibrary().addOrUpdateBook(bCopy);
        mp_library->save();
        emit(mp_library->booksChanged());
        return QMap<std::string, std::string>();
    }*/

    auto map = mp_background_downloader->getDownloadStatus(b.getDownloadId());
    if (map.value("status") == "complete") {
        QString tmp(QString::fromStdString(map.value("path")));
        kiwix::Book bCopy(b);
        bCopy.setPath(QDir::toNativeSeparators(tmp).toStdString());
        bCopy.setDownloadId("");
        bCopy.setPathValid(true);
        // removing book url so that download link in kiwix-serve is not displayed.
        bCopy.setUrl("");
        mp_library->getKiwixLibrary().addOrUpdateBook(bCopy);
        mp_library->save();
        mp_library->bookmarksChanged();
        if (!m_local) {
            emit(oneBookChanged(id));
        } else {
            emit(mp_library->booksChanged());
        }
        emit(backgroundCompleteDownload(QString::fromStdString(b.getDownloadId())));
    }

    QStringList values;
    for (auto& key : keys) {
        values.append(QString::fromStdString(map.value(key.toStdString())));
    }
    return values;
}

QString ContentManager::startDownloadBook(const QString &bookId)
{
    std::cerr << "request for book id " << bookId.toStdString() << '\n';
    const auto& book = [&]()->const kiwix::Book& {
        try {
            return m_remoteLibrary.getBookById(bookId.toStdString());
        } catch (...) {
            return mp_library->getBookById(bookId);
        }
    }();
    std::cerr << "found book id " << bookId.toStdString() << '\n';
    auto downloadPath = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    QStorageInfo storage(downloadPath);
    auto bytesAvailable = storage.bytesAvailable();
    if (bytesAvailable == -1 || book.getSize() > (unsigned long long) bytesAvailable) {
        return "storage_error";
    }
    auto booksList = mp_library->getBookIds();
    for (auto b : booksList)
        if (b.toStdString() == book.getId())
            return "";

    std::cerr << "emitting background request for book id " << bookId.toStdString() << '\n';

    emit(backgroundStartDownload(bookId, QString::fromStdString(book.getUrl()), downloadPath));
    return "";
}

QString ContentManager::downloadBook(const QString& bookId, const QString& did)
{
    const auto& book = [&]()->const kiwix::Book& {
        try {
            return m_remoteLibrary.getBookById(bookId.toStdString());
        } catch (...) {
            return mp_library->getBookById(bookId);
        }
    }();
    std::cerr << "got book with id " << bookId.toStdString() << '\n';
    kiwix::Book bookCopy(book);
    bookCopy.setDownloadId(did.toStdString());
    mp_library->addBookToLibrary(bookCopy);
    mp_library->save();
    emit(oneBookChanged(bookId));
    return did;
}

void ContentManager::eraseBookFilesFromComputer(const QString dirPath, const QString fileName)
{
    if (fileName == "*") {
        return;
    }
    QDir dir(dirPath, fileName);
    for(const QString& file: dir.entryList()) {
        dir.remove(file);
    }
}

void ContentManager::eraseBook(const QString& id)
{
    auto tabBar = KiwixApp::instance()->getTabWidget();
    tabBar->closeTabsByZimId(id);
    kiwix::Book book = mp_library->getBookById(id);
    QString dirPath = QString::fromStdString(kiwix::removeLastPathElement(book.getPath()));
    QString fileName = QString::fromStdString(kiwix::getLastPathElement(book.getPath())) + "*";
    eraseBookFilesFromComputer(dirPath, fileName);
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
    auto& b = mp_library->getBookById(id);
    emit(backgroundPauseDownload(QString::fromStdString(b.getDownloadId())));
}

void ContentManager::resumeBook(const QString& id)
{
    auto& b = mp_library->getBookById(id);
    emit(backgroundResumeDownload(QString::fromStdString(b.getDownloadId())));
}

void ContentManager::startCancelBook(const QString& id)
{
    auto& b = mp_library->getBookById(id);
    QMap<std::string, std::string> map = mp_background_downloader->getDownloadStatus(b.getDownloadId());
    emit(backgroundCancelDownload(id, QString::fromStdString(b.getDownloadId())));
}

void ContentManager::completeCancelBook(const QString& bookId, const QString& path)
{
    auto stdPath = path.toStdString();
    QString dirPath = QString::fromStdString(kiwix::removeLastPathElement(stdPath));
    QString filename = QString::fromStdString(kiwix::getLastPathElement(stdPath)) + "*";
    eraseBookFilesFromComputer(dirPath, filename);
    mp_library->removeBookFromLibraryById(bookId);
    mp_library->save();
    emit(oneBookChanged(bookId));
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

void ContentManager::setCurrentContentTypeFilter(QList<ContentTypeFilter*>& contentTypeFilters)
{
    m_contentTypeFilters = contentTypeFilters;
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
    } catch (std::runtime_error&) {}
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
    std::vector<std::string> acceptTags, rejectTags;
    if (m_categoryFilter != "all" && m_categoryFilter != "other") {
        acceptTags.push_back("_category:"+m_categoryFilter.toStdString());
    }
    if (m_categoryFilter == "other") {
        for (auto& category: S_CATEGORIES) {
            if (category.first != "other" && category.first != "all") {
                rejectTags.push_back("_category:"+category.first.toStdString());
            }
        }
    }

    for (auto &contentTypeFilter : m_contentTypeFilters) {
        auto state = contentTypeFilter->checkState();
        auto filter = contentTypeFilter->getName();
        if (state == Qt::PartiallyChecked) {
            acceptTags.push_back("_" + filter.toStdString() +":yes");
        } else if (state == Qt::Checked) {
            acceptTags.push_back("_" + filter.toStdString() +":no");
        }
    }

    filter.acceptTags(acceptTags);
    filter.rejectTags(rejectTags);
    filter.query(m_searchQuery.toStdString());
    if (m_currentLanguage != "*")
        filter.lang(m_currentLanguage.toStdString());

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
