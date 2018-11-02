#include "contentmanager.h"

#include "kiwixapp.h"
#include <kiwix/common/networkTools.h>
#include <kiwix/common/otherTools.h>
#include <kiwix/manager.h>

#include <QDebug>
#include <QUrlQuery>
#include <QUrl>

ContentManager::ContentManager(Library* library, kiwix::Downloader* downloader, QObject *parent)
    : QObject(parent),
      mp_library(library),
      mp_downloader(downloader)
{
    // mp_view will be passed to the tab who will take ownership,
    // so, we don't need to delete it.
    mp_view = new ContentManagerView();
    mp_view->registerObject("contentManager", this);
    mp_view->setHtml();
    setCurrentLanguage(QLocale().name().split("_").at(0));
    connect(mp_library, &Library::booksChanged, this, [=]() {emit(this->booksChanged());});
    connect(this, &ContentManager::remoteParamsChanged, this, &ContentManager::updateRemoteLibrary);
    connect(this, &ContentManager::booksChanged, this, [=]() {if (!m_local) this->updateRemoteLibrary(); });
}


void ContentManager::setLocal(bool local) {
    if (local == m_local) {
        return;
    }
    m_local = local;
    m_currentPage = 0;
    emit(remoteParamsChanged());
    emit(booksChanged());
}

#define ADD_V(KEY, METH) {if(key==KEY) values.append(QString::fromStdString((b.METH())));}
QStringList ContentManager::getBookInfos(QString id, const QStringList &keys)
{
    QStringList values;
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
    }
    kiwix::Book& b = [=]()->kiwix::Book& {
        try {
            return mp_library->getBookById(id);
        } catch (...) {
            return m_remoteLibrary.getBookById(id.toStdString());
        }
    }();

    for(auto& key: keys){
        ADD_V("id", getId);
        ADD_V("path", getPath);
        ADD_V("indexPath", getIndexPath);
        ADD_V("title", getTitle);
        ADD_V("description", getDescription);
        ADD_V("language", getLanguage);
        ADD_V("creator", getCreator);
        ADD_V("publisher", getPublisher);
        ADD_V("date", getDate);
        ADD_V("url", getUrl);
        ADD_V("name", getName);
        ADD_V("tags", getTags);
        ADD_V("origId", getOrigId);
        ADD_V("faviconMimeType", getFaviconMimeType);
        ADD_V("downloadId", getDownloadId);
        if (key == "favicon") {
            auto s = b.getFavicon();
            values.append(QByteArray::fromStdString(s).toBase64());
        }
        if (key == "size") {
            values.append(QString::number(b.getSize()));
        }
        if (key == "articleCount") {
            values.append(QString::number(b.getArticleCount()));
        }
        if (key == "mediaCount") {
            values.append(QString::number(b.getMediaCount()));
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
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
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
        b.setPath(d->getPath());
        b.setDownloadId("");
        mp_library->save();
        emit(mp_library->booksChanged());
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
    auto download = mp_downloader->startDownload(book.getUrl());
    book.setDownloadId(download->getDid());
    mp_library->addBookToLibrary(book);
    mp_library->save();
    emit(mp_library->booksChanged());
    emit(booksChanged());
    return QString::fromStdString(download->getDid());
}

QStringList ContentManager::getDownloadIds()
{
    QStringList list;
    if (!mp_downloader)
        return list;
    for(auto& id: mp_downloader->getDownloadIds()) {
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
}

#define CATALOG_HOST "library.kiwix.org"
#define CATALOG_PORT 80
void ContentManager::updateRemoteLibrary() {
    QUrlQuery query;
    query.addQueryItem("lang", m_currentLanguage);
    query.addQueryItem("count", QString::number(m_booksPerPage));
    query.addQueryItem("start", QString::number(getStartBookIndex()));
    QUrl url;
    url.setScheme("http");
    url.setHost(CATALOG_HOST);
    url.setPort(CATALOG_PORT);
    url.setPath("/catalog/search");
    url.setQuery(query);
    qInfo() << "Downloading" << url;
    kiwix::Manager manager(&m_remoteLibrary);
    try {
        auto allContent = kiwix::download(url.toString().toStdString());
        manager.readOpds(allContent, CATALOG_HOST);
        m_totalBooks = manager.m_totalBooks;
    } catch (runtime_error&) {}
}

QStringList ContentManager::getBookIds() {
    if (m_local) {
        return mp_library->getBookIds().mid(getStartBookIndex(), m_booksPerPage);
    } else {
        auto bookIds = m_remoteLibrary.getBooksIds();
        QStringList list;
        for(auto i=0; i<m_booksPerPage; i++) {
            try{
                list.append(QString::fromStdString(bookIds.at(getStartBookIndex()+i)));
            } catch (out_of_range& e) {
                break;
            }
        }
        return list;
    }
}
