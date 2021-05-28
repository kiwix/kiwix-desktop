#include "suggestionlistworker.h"
#include "kiwixapp.h"

SuggestionListWorker::SuggestionListWorker(const QString& text, int token, QObject *parent)
: QThread(parent),
  m_text(text),
  m_token(token)
{
}

void SuggestionListWorker::run()
{
    QStringList suggestionList;
    QVector<QUrl> urlList;

    WebView *current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if(!current)
        return;
    auto qurl = current->url();
    auto currentZimId = qurl.host().split(".")[0];
    auto reader = KiwixApp::instance()->getLibrary()->getReader(currentZimId);
    QUrl url;
    url.setScheme("zim");
    if (reader) {
        url.setHost(currentZimId + ".zim");
        kiwix::SuggestionsList_t suggestions;
        reader->searchSuggestionsSmart(m_text.toStdString(), 15, suggestions);
        for (auto& suggestion: suggestions) {
            QString path = QString("/") + QString::fromStdString(suggestion.getPath());
            url.setPath(path);
            suggestionList.append(QString::fromStdString(suggestion.getTitle()));
            urlList.append(url);
        }
    }
    QUrlQuery query;
    url.setPath("");
    if (reader) {
        // The host is used to determine the currentZimId
        // The content query item is used to know in which zim search (as for kiwix-serve)
        url.setHost(currentZimId + ".search");
        query.addQueryItem("content", currentZimId);
    } else {
        // We do not allow multi zim search for now.
        // We don't have a correct UI to select on which zim search,
        // how to display results, ...
        //url.setHost("library.search");
    }
    query.addQueryItem("pattern", m_text);
    url.setQuery(query);
    suggestionList.append(m_text + " (" + gt("fulltext-search") + ")");
    urlList.append(url);
    emit(searchFinished(suggestionList, urlList, m_token));
}
