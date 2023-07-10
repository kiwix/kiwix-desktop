#include "opdsrequestmanager.h"
#include "static_content.h"
#include "kiwixapp.h"

OpdsRequestManager::OpdsRequestManager()
{
}

#define CATALOG_HOST "library.kiwix.org"
#define CATALOG_PORT 443
void OpdsRequestManager::doUpdate(const QString& currentLanguage, const QString& categoryFilter)
{
    QUrlQuery query;

    // Service worker ZIM files are not (yet) supported
    QStringList excludeTags("_sw:yes");

    // Add filter by language (if necessary)
    if (currentLanguage != "*") {
        query.addQueryItem("lang", currentLanguage);
    }

    // Request all results (no pagination)
    query.addQueryItem("count", QString::number(-1));

    // Add filter by category (if necessary)
    if (categoryFilter != "all" && categoryFilter != "other") {
        query.addQueryItem("tag", "_category:"+categoryFilter);
    }

    // Add "special negative" filter for "other" category (if necessary)
    if (categoryFilter == "other") {
        for (auto& category: S_CATEGORIES) {
            if (category.first != "other" && category.first != "all") {
                excludeTags += "_category:"+category.first;
            }
        }
    }
    query.addQueryItem("notag", excludeTags.join(";"));

    auto mp_reply = opdsResponseFromPath("/catalog/search", query);
    connect(mp_reply, &QNetworkReply::finished, this, [=]() {
        receiveContent(mp_reply);
    });
}

QNetworkReply* OpdsRequestManager::opdsResponseFromPath(const QString &path, const QUrlQuery &query)
{
    QUrl url;
    url.setScheme("https");
    url.setHost(CATALOG_HOST);
    url.setPort(CATALOG_PORT);
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
