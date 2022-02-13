#include "kiwixapp.h"
#include "urlschemehandler.h"
#include "blobbuffer.h"
#include <QDebug>
#include <QWebEngineUrlRequestJob>
#include <QTextStream>
#include <iostream>

#include <kiwix/search_renderer.h>
#include <kiwix/name_mapper.h>

UrlSchemeHandler::UrlSchemeHandler()
{
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
    auto reader = library->getReader(zim_id);
    if ( reader == nullptr) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    try {
        kiwix::Entry entry = reader->getEntryFromPath(url);
        if (entry.isRedirect()) {
            entry = entry.getFinalEntry();
            auto path = QString("/") + QString::fromStdString(entry.getPath());
            qurl.setPath(path);
            request->redirect(qurl);
            return;
        }

        BlobBuffer* buffer = new BlobBuffer(entry.getBlob());
        auto mimeType = QByteArray::fromStdString(entry.getMimetype());
        mimeType = mimeType.split(';')[0];
        connect(request, &QObject::destroyed, buffer, &QObject::deleteLater);
        request->reply(mimeType, buffer);
    } catch (kiwix::NoEntry&) {
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
          std::string content= book.getFavicon();
          std::string mimeType = book.getFaviconMimeType();
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

    auto end = start + pageLength;

    auto searcher = app->getLibrary()->getSearcher(bookId);
    searcher->search(searchQuery, start, end);

    IdNameMapper nameMapper;
    kiwix::SearchRenderer renderer(searcher.get(), &nameMapper);
    renderer.setSearchPattern(searchQuery);
    renderer.setSearchContent(bookId.toStdString());
    renderer.setProtocolPrefix("zim://");
    renderer.setSearchProtocolPrefix("zim://" + host.toStdString() + "/?");
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
