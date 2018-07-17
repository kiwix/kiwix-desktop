#include "kiwixapp.h"
#include "kiwixschemehandler.h"
#include "blobbuffer.h"
#include <QDebug>
#include <QWebEngineUrlRequestJob>
#include <QTextStream>
#include <iostream>

KiwixSchemeHandler::KiwixSchemeHandler()
{

}


void
KiwixSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    std::cout << "Handling request " << request->requestUrl().toString().toUtf8().constData() << std::endl;
    auto qurl = request->requestUrl();
    std::string url = qurl.path().toUtf8().constData();
    std::cout << "Url is " << url << std::endl;
    if (url[0] == '/')
        url = url.substr(1);
    auto library = static_cast<KiwixApp*>(KiwixApp::instance())->getLibrary();
    auto zim_id = qurl.host();
    auto reader = library->getReader(zim_id);
    if ( reader == nullptr) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    kiwix::Entry entry;
    try {
        entry = reader->getEntryFromPath(url);
    } catch (kiwix::NoEntry& e) {
        url = "A/" + url;
        std::cout << "Url is " << url << std::endl;
        try {
            entry = reader->getEntryFromPath(url);
        } catch (kiwix::NoEntry& e) {
            request->fail(QWebEngineUrlRequestJob::UrlNotFound);
            return;
        }
    }
    try {
        entry = entry.getFinalEntry();
    } catch (kiwix::NoEntry& e) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    BlobBuffer* buffer = new BlobBuffer(entry.getBlob());
    std::cout << "  mimetype : '" << entry.getMimetype() << "'" << std::endl;
    auto mimeType = QByteArray::fromStdString(entry.getMimetype());
    connect(buffer, &QIODevice::aboutToClose, buffer, &QObject::deleteLater);
    request->reply(mimeType, buffer);
}
