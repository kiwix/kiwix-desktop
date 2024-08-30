#include "urlschemehandler.h"
#include "kiwixapp.h"
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

zim::Entry getArchiveEntryFromUrl(const zim::Archive& archive, const QUrl& url)
{
  std::string path = url.path().toUtf8().constData();
  if (path[0] == '/')
    path = path.substr(1);

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
    auto library = KiwixApp::instance()->getLibrary();
    auto zim_id = qurl.host();
    zim_id.resize(zim_id.length()-4);
    std::shared_ptr<zim::Archive> archive;
    try {
      archive = library->getArchive(zim_id);
    } catch (std::out_of_range& e) {
      replyZimNotFoundPage(request, zim_id);
      return;
    }
    try {
        auto entry = getArchiveEntryFromUrl(*archive, qurl);
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
    } catch (const zim::ZimFileFormatError&) {
      replyBadZimFilePage(request, zim_id);
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


namespace
{

struct SearchResultsWithEstimatedMatchCount
{
    std::shared_ptr<zim::SearchResultSet> results;
    int estimatedMatchCount = 0;
};

SearchResultsWithEstimatedMatchCount getSearchResults(zim::Search& s, int start, int pageLength)
{
    SearchResultsWithEstimatedMatchCount r;
    r.estimatedMatchCount = s.getEstimatedMatches();
    r.results = std::make_shared<zim::SearchResultSet>(s.getResults(start, pageLength));
    return r;
}

} // unnamed namespace

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

    SearchResultsWithEstimatedMatchCount searchResult;
    try {
        searchResult = getSearchResults(*search, start, pageLength);
    } catch (...) {
        request->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
    }

    kiwix::SearchRenderer renderer(
        *searchResult.results,
        start,
        searchResult.estimatedMatchCount);
    renderer.setSearchPattern(searchQuery);
    renderer.setSearchBookQuery("content="+bookId.toStdString());
    renderer.setProtocolPrefix("zim://");
    renderer.setSearchProtocolPrefix("zim://" + host.toStdString() + "/");
    renderer.setPageLength(pageLength);
    IdNameMapper mapper;
    auto content = renderer.getHtml(mapper, nullptr);
    QBuffer *buffer = new QBuffer;
    buffer->setData(content.data(), content.size());
    connect(request, &QObject::destroyed, buffer, &QObject::deleteLater);
    request->reply("text/html", buffer);
}

namespace
{

QString completeHtml(const QString& htmlBodyContent)
{
    const QString htmlHead = R"(<head><meta charset="utf-8"></head>)";
    const QString fullHtml = "<!DOCTYPE html><html>"
                             + htmlHead
                             + "<body>" + htmlBodyContent + "</body>"
                             + "</html>";
    return fullHtml;
}

void
sendHtmlResponse(QWebEngineUrlRequestJob *request, const QString& htmlBodyContent)
{
    QBuffer *buffer = new QBuffer;
    buffer->open(QIODevice::WriteOnly);
    buffer->write(completeHtml(htmlBodyContent).toStdString().c_str());
    buffer->close();

    QObject::connect(request, SIGNAL(destroyed()), buffer, SLOT(deleteLater()));
    request->reply("text/html", buffer);
}

} // unnamed namespace

void
UrlSchemeHandler::replyZimNotFoundPage(QWebEngineUrlRequestJob *request,
                                       const QString &zimId)
{
    QString path = "N/A", name = "N/A";
    try
    {
        auto& book = KiwixApp::instance()->getLibrary()->getBookById(zimId);
        path = QString::fromStdString(book.getPath());
        name = QString::fromStdString(book.getName());
    }
    catch (...) { /* Blank */ }

    QString contentHtml = "<section><div>"
                          "<h1>" +
                          gt("file-not-found-title") +
                          "</h1>"
                          "<p>" +
                          gt("file-not-found-text") +
                          "</p>"
                          "<p>" +
                          gt("zim-id") + ": <b>" + zimId +
                          "</b></p>"
                          "<p>" +
                          gt("zim-name") + ": <b>" + name +
                          "</b></p>"
                          "<p>" +
                          gt("zim-path") + ": <b>" + path +
                          "</b></p>"
                          "</div></section>";

    sendHtmlResponse(request, contentHtml);
}

void
UrlSchemeHandler::replyBadZimFilePage(QWebEngineUrlRequestJob *request,
                                  const QString &zimId)
{
    const auto& book = KiwixApp::instance()->getLibrary()->getBookById(zimId);
    const QString path = QString::fromStdString(book.getPath());
    const QString name = QString::fromStdString(book.getName());
    const QString zimEntryPath = request->requestUrl().path();

    QString contentHtml = "<section><div>"
                          "<h1>" + gt("bad-zim-file-error-page-title") + "</h1>"
                          "<p>"  + gt("bad-zim-file-error-page-text") + "</p>"
                          "<p>"  + gt("zim-id")   + ": <b>" + zimId + "</b></p>"
                          "<p>"  + gt("zim-name") + ": <b>" + name  + "</b></p>"
                          "<p>"  + gt("zim-path") + ": <b>" + path  + "</b></p>"
                          "<p>" +  gt("zim-entry-path") + ": <b>" + zimEntryPath + "</b></p>"
                          "</div></section>";

    sendHtmlResponse(request, contentHtml);
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
