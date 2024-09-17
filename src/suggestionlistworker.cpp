#include "suggestionlistworker.h"
#include "kiwixapp.h"
#include <zim/suggestion.h>

SuggestionListWorker::SuggestionListWorker(const QString& text, int token, int start, QObject *parent)
: QThread(parent),
  m_text(text),
  m_token(token),
  m_start(start)
{
}

void SuggestionListWorker::run()
{
    QStringList suggestionList;
    QVector<QUrl> urlList;

    auto app = KiwixApp::instance();
    auto currentZimId = app->getSearchBar().getMultiZimButtom().getZimId();
    try {
        auto archive = app->getLibrary()->getArchive(currentZimId);
        QUrl url;
        url.setScheme("zim");
        url.setHost(currentZimId + ".zim");
        auto prefix = m_text.toStdString();
        auto suggestionSearcher = zim::SuggestionSearcher(*archive);
        auto suggestionSearch = suggestionSearcher.suggest(prefix);
        const auto suggestions = suggestionSearch.getResults(m_start, getFetchSize());
        for (auto current : suggestions) {
            QString path = QString("/") + QString::fromStdString(current.getPath());
            url.setPath(path);
            suggestionList.append(QString::fromStdString(current.getTitle()));
            urlList.append(url);
        }

        // Propose fulltext search
        url.setPath("");
        bool hasFullText = archive->hasFulltextIndex();
        if (hasFullText) {
            // The host is used to determine the currentZimId
            // The content query item is used to know in which zim search (as for kiwix-serve)
            url.setHost(currentZimId + ".search");
            QUrlQuery query;
            query.addQueryItem("content", currentZimId);
            query.addQueryItem("pattern", m_text);
            url.setQuery(query);
            suggestionList.append(m_text + " (" + gt("fulltext-search") + ")");
            urlList.append(url);
        }
        emit(searchFinished(suggestionList, urlList, hasFullText, m_token));
    } catch (std::out_of_range& e) {
        // Impossible to find the requested archive (bug ?)
        // We could propose a suggestion to do multi-zim search with:
        // url.setHost("library.search");
        // but we don't have a correct UI to select on which zim search, how to display results, ...
        // So do nothing for now
        emit(searchFinished(suggestionList, urlList, false, m_token));
    }
}
