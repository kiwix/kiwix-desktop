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
    auto qurl = request->requestUrl();
    std::string url = qurl.path().toUtf8().constData();
    qDebug() << "Handling request" << qurl;
    if (url[0] == '/')
        url = url.substr(1);
    auto library = KiwixApp::instance()->getLibrary();
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
    auto mimeType = QByteArray::fromStdString(entry.getMimetype());
    connect(buffer, &QIODevice::aboutToClose, buffer, &QObject::deleteLater);
    request->reply(mimeType, buffer);
}
