#include "contentmanager.h"

#include "kiwixapp.h"

#include <QDebug>

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
    connect(mp_library, &Library::booksChanged, this, [=]() {emit(this->booksChanged());});
}


#define ADD_V(KEY, METH) {if(key==KEY) values.append(QString::fromStdString((b.METH())));}
QStringList ContentManager::getBookInfos(QString id, const QStringList &keys)
{
    QStringList values;
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
    }
    auto& b = mp_library->getBookById(id);
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
    if (d->getStatus() == kiwix::Download::COMPLETE) {
        b.setPath(d->getPath());
        b.setDownloadId("");
        mp_library->save();
        emit(mp_library->booksChanged());
    }
    for(auto& key: keys){
        ADD_V("id", getDid);
        if(key == "status") {
            switch(d->getStatus()){
            case kiwix::Download::ACTIVE:
                values.append("active");
                break;
            case kiwix::Download::WAITING:
                values.append("waiting");
                break;
            case kiwix::Download::PAUSED:
                values.append("paused");
                break;
            case kiwix::Download::ERROR:
                values.append("error");
                break;
            case kiwix::Download::COMPLETE:
                values.append("completed");
                break;
            case kiwix::Download::REMOVED:
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
    auto& book = mp_library->getBookById(id);
    auto download = mp_downloader->startDownload(book.getUrl());
    book.setDownloadId(download->getDid());
    return QString::fromStdString(download->getDid());
    emit(booksChanged());
}

QStringList ContentManager::getDownloadIds()
{
    QStringList list;
    for(auto& id: mp_downloader->getDownloadIds()) {
        list.append(QString::fromStdString(id));
    }
    return list;
}
