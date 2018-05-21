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
    std::string url = request->requestUrl().path().toUtf8().constData();
    zim::Article art;
    std::cout << "Url is " << url << std::endl;
    auto reader = static_cast<KiwixApp*>(KiwixApp::instance())->getReader();
    if ( !reader->getArticleObjectByDecodedUrl(url, art))
    {
        url = "A/" + url;
        if (!reader->getArticleObjectByDecodedUrl(url, art))
        {
            request->fail(QWebEngineUrlRequestJob::UrlNotFound);
            return;
        }
    }

    BlobBuffer* buffer = new BlobBuffer(art.getData());
    std::cout << "  mimetype : " << art.getMimeType() << std::endl;
    auto mimeType = QByteArray::fromRawData(art.getMimeType().data(), art.getMimeType().size());
    connect(buffer, &QIODevice::aboutToClose, buffer, &QObject::deleteLater);
    request->reply(mimeType, buffer);
}
