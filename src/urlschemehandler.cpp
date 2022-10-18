#include "kiwixapp.h"
#include "urlschemehandler.h"
#include "blobbuffer.h"
#include <QDebug>
#include <QWebEngineUrlRequestJob>
#include <QTextStream>
#include <iostream>

#include <kiwix/search_renderer.h>
#include <kiwix/name_mapper.h>
#include <zim/search.h>
#include <zim/entry.h>
#include <zim/item.h>
#include <zim/error.h>


UrlSchemeHandler::UrlSchemeHandler()
{
}

zim::Entry getEntryFromPath(const zim::Archive& archive, const std::string& path)
{
  try {
    return archive.getEntryByPath(path);
  } catch (zim::EntryNotFound& e) {
    if (path.empty() || path == "/") {
      return archive.getMainEntry();
    }
  }
  throw zim::EntryNotFound("Cannot find entry for non empty path");
}

void
UrlSchemeHandler::handleContentRequest(QWebEngineUrlRequestJob *request)
{
    auto qurl = request->requestUrl();
    std::string url = qurl.path().toUtf8().constData();
    if (url[0] == '/')
        url = url.substr(1);
    auto library = KiwixApp::instance()->getLibrary();
    auto zim_id = qurl.host();
    zim_id.resize(zim_id.length()-4);
    std::shared_ptr<zim::Archive> archive;
    try {
      archive = library->getArchive(zim_id);
    } catch (std::out_of_range& e) {
      request->fail(QWebEngineUrlRequestJob::UrlNotFound);
      return;
    }
    try {
        auto entry = getEntryFromPath(*archive, url);
        auto item = entry.getItem(true);
        if (entry.isRedirect()) {
            auto path = QString("/") + QString::fromStdString(item.getPath());
            qurl.setPath(path);
            request->redirect(qurl);
            return;
        }

        BlobBuffer* buffer = new BlobBuffer(item.getData(0));
        auto mimeType = QByteArray::fromStdString(item.getMimetype());
        mimeType = mimeType.split(';')[0];
        connect(request, &QObject::destroyed, buffer, &QObject::deleteLater);
        request->reply(mimeType, buffer);
    } catch (zim::EntryNotFound&) {
      request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }
}

void
UrlSchemeHandler::handleMetaRequest(QWebEngineUrlRequestJob* request)
{
    auto qurl = request->requestUrl();
    auto host = qurl.host();
    auto parts = host.split('.');
    auto zimId = parts[0];
    auto metaName = parts[1];

    if (metaName == "favicon") {
        try {
          auto library = KiwixApp::instance()->getLibrary();
          auto book = library->getBookById(zimId);
          auto illustration = book.getIllustration(48);
          std::string content = illustration->getData();
          std::string mimeType = illustration->mimeType;
          QBuffer* buffer = new QBuffer;
          buffer->setData(content.data(), content.size());
          connect(request, &QObject::destroyed, buffer, &QObject::deleteLater);
          request->reply(QByteArray::fromStdString(mimeType), buffer);
          return;
        } catch (...) {}
    }
    request->fail(QWebEngineUrlRequestJob::UrlNotFound);
}


class IdNameMapper : public kiwix::NameMapper {
  std::string getNameForId(const std::string& id) const { return id + ".zim"; }
  std::string getIdForName(const std::string& id) const { return id.substr(0, id.size()-4); }
};


void
UrlSchemeHandler::handleSearchRequest(QWebEngineUrlRequestJob* request)
{
    auto qurl = request->requestUrl();
    auto app = KiwixApp::instance();
    auto host = qurl.host();
    auto bookId = host.split('.')[0];
    qInfo() << "Handling request" << qurl;
    QUrlQuery query(qurl.query());
    if (bookId == "library") {
      bookId = query.queryItemValue("content");
    }
    auto searchQuery = query.queryItemValue("pattern").toStdString();
    int start = 0;
    bool ok;
    int temp = query.queryItemValue("start").toInt(&ok);
    if (ok)
      start = temp;
    int pageLength = 25;
    temp = query.queryItemValue("pageLength").toInt(&ok);
    if (ok)
      pageLength = temp;

    std::shared_ptr<zim::Search> search;
    try {
        auto searcher = app->getLibrary()->getSearcher(bookId);
        search = std::make_shared<zim::Search>(searcher->search(searchQuery));
    } catch(...) {
        request->fail(QWebEngineUrlRequestJob::UrlInvalid);
        return;
    }
    auto nameMapper = std::make_shared<IdNameMapper>();
    kiwix::SearchRenderer renderer(
        search->getResults(start, pageLength),
        nameMapper,
        search->getEstimatedMatches(),
        start);
    renderer.setSearchPattern(searchQuery);
    renderer.setSearchBookQuery("content="+bookId.toStdString());
    renderer.setProtocolPrefix("zim://");
    renderer.setSearchProtocolPrefix("zim://" + host.toStdString() + "/");
    renderer.setPageLength(pageLength);
    auto content = renderer.getHtml();
    QBuffer *buffer = new QBuffer;
    buffer->setData(content.data(), content.size());
    connect(request, &QObject::destroyed, buffer, &QObject::deleteLater);
    request->reply("text/html", buffer);
}

void
UrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    auto qurl = request->requestUrl();
    auto host = qurl.host();
    if (host.endsWith(".zim")) {
        handleContentRequest(request);
    } else if (host.endsWith(".meta")) {
        handleMetaRequest(request);
    } else if (host.endsWith(".search")) {
        handleSearchRequest(request);
    } else {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }
}
