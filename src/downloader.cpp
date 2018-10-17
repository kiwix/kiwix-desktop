#include "downloader.h"

Downloader::Downloader(Library* library, QObject *parent)
    : QObject(parent),
      mp_library(library)
{

}

Downloader::~Downloader()
{
    m_downloader.close();
}

QStringList Downloader::getDownloadIds()
{
    QStringList list;
    for(auto& id: m_downloader.getDownloadIds()) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

QString Downloader::downloadBook(const QString& id) {
    auto& book = mp_library->getBookById(id);
    auto download = m_downloader.startDownload(book.getUrl());
    book.setDownloadId(download->getDid());
    return QString::fromStdString(download->getDid());
}

#define ADD_V(KEY, METH) {if(key==KEY) {values.append(QString::fromStdString((d->METH()))); continue;}}
QStringList Downloader::updateDownloadInfos(QString id, const QStringList &keys)
{
    QStringList values;
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
    }
    auto& b = mp_library->getBookById(id);
    kiwix::Download* d;
    try {
        d = m_downloader.getDownload(b.getDownloadId());
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

int Downloader::getNbDownload() {
    return m_downloader.getNbDownload();
}
