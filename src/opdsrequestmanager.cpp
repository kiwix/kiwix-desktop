#include "opdsrequestmanager.h"
#include "kiwixapp.h"

OpdsRequestManager::OpdsRequestManager()
{
}

QString OpdsRequestManager::getCatalogHost()
{
    const char* const envVarVal = getenv("KIWIX_CATALOG_HOST");
    return envVarVal
         ? envVarVal
         : "library.kiwix.org";
}

int OpdsRequestManager::getCatalogPort()
{
    const char* const envVarVal = getenv("KIWIX_CATALOG_PORT");
    return envVarVal
         ? atoi(envVarVal)
         : 443;
}

void OpdsRequestManager::doUpdate(const QString& currentLanguage, const QString& categoryFilter)
{
    QUrlQuery query;

    // Service worker ZIM files are not (yet) supported
    QStringList excludeTags("_sw:yes");

    // Add filter by language (if necessary)
    if (currentLanguage != "") {
        query.addQueryItem("lang", currentLanguage);
    }

    // Request all results (no pagination)
    query.addQueryItem("count", QString::number(-1));

    // Add filter by category (if necessary)
    if (categoryFilter != "") {
        query.addQueryItem("category", categoryFilter);
    }

    auto mp_reply = opdsResponseFromPath("/catalog/search", query);
    connect(mp_reply, &QNetworkReply::finished, this, [=]() {
        receiveContent(mp_reply);
    });
}

QNetworkReply* OpdsRequestManager::opdsResponseFromPath(const QString &path, const QUrlQuery &query)
{
    QUrl url;
    const int port = getCatalogPort();
    url.setScheme(port == 443 ? "https" : "http");
    url.setHost(getCatalogHost());
    url.setPort(port);
    url.setPath(path);
    url.setQuery(query);
    qInfo() << "Downloading" << url.toString(QUrl::FullyEncoded);
    QNetworkRequest request(url);
    return m_networkManager.get(request);
}

void OpdsRequestManager::getLanguagesFromOpds()
{
    auto mp_reply = opdsResponseFromPath("/catalog/v2/languages");
    connect(mp_reply, &QNetworkReply::finished, this, [=]() {
        receiveLanguages(mp_reply);
    });
}

void OpdsRequestManager::getCategoriesFromOpds()
{
    auto mp_reply = opdsResponseFromPath("/catalog/v2/categories");
    connect(mp_reply, &QNetworkReply::finished, this, [=]() {
        receiveCategories(mp_reply);
    });
}

QString replyContent(QNetworkReply *mp_reply)
{
    QString content;
    if (mp_reply->error() != QNetworkReply::OperationCanceledError) {
        content = mp_reply->readAll().data();
    }
    mp_reply->deleteLater();
    mp_reply = nullptr;
    return content;
}

void OpdsRequestManager::receiveLanguages(QNetworkReply *mp_reply)
{
    emit(languagesReceived(replyContent(mp_reply)));
}

void OpdsRequestManager::receiveCategories(QNetworkReply *mp_reply)
{
    emit(categoriesReceived(replyContent(mp_reply)));
}

void OpdsRequestManager::receiveContent(QNetworkReply *mp_reply)
{
    emit(requestReceived(replyContent(mp_reply)));
}
