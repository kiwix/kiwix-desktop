#include "opdsrequestmanager.h"
#include "static_content.h"
#include "kiwixapp.h"

OpdsRequestManager::OpdsRequestManager()
: mp_reply(nullptr)
{
}

#define CATALOG_HOST "library.kiwix.org"
#define CATALOG_PORT 80
void OpdsRequestManager::doUpdate(const QString& currentLanguage, const QString& categoryFilter)
{
    QUrlQuery query;
    if (currentLanguage != "*") {
        query.addQueryItem("lang", currentLanguage);
    }
    query.addQueryItem("count", QString::number(0));
    if (categoryFilter != "all" && categoryFilter != "other") {
        query.addQueryItem("tag", "_category:"+categoryFilter);
    }

    if (categoryFilter == "other") {
        QStringList excludeTags;
        for (auto& category: S_CATEGORIES) {
            if (category.first != "other" && category.first != "all") {
		excludeTags += "_category:"+category.first;
            }
        }
        query.addQueryItem("notag", excludeTags.join(";"));
    }


    QUrl url;
    url.setScheme("http");
    url.setHost(CATALOG_HOST);
    url.setPort(CATALOG_PORT);
    url.setPath("/catalog/search");
    url.setQuery(query);

    qInfo() << "Downloading" << url.toString(QUrl::FullyEncoded);

    QNetworkRequest request(url);
    if (mp_reply) {
        mp_reply->abort();
    }
    mp_reply = m_networkManager.get(request);
    connect(mp_reply, &QNetworkReply::finished, this, &OpdsRequestManager::receiveContent);
}

void OpdsRequestManager::receiveContent()
{
    if (mp_reply->error() != QNetworkReply::OperationCanceledError) {
        QString content = mp_reply->readAll().data();
        emit(requestReceived(content));
    }
    mp_reply->deleteLater();
    mp_reply = nullptr;
}
