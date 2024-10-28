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
    QList<SuggestionData> suggestionList;

    const auto app = KiwixApp::instance();
    const auto selectedIdList = app->getSearchBar().getMultiZimButton().getZimIds();
    
    /* TODO: re-implement this after introducing the actual Multi-Zim. */
    const auto currentZimId = selectedIdList[0];
    try {
        const auto archive = app->getLibrary()->getArchive(currentZimId);
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
            const auto text = QString::fromStdString(current.getTitle());
            suggestionList.append({text, url});
        }

        // Propose fulltext search
        url.setPath("");
        if (archive->hasFulltextIndex()) {
            // The host is used to determine the currentZimId
            // The content query item is used to know in which zim search (as for kiwix-serve)
            url.setHost(currentZimId + ".search");
            QUrlQuery query;
            query.addQueryItem("content", currentZimId);
            query.addQueryItem("pattern", m_text);
            url.setQuery(query);
            const auto text = m_text + " (" + gt("fulltext-search") + ")";
            suggestionList.append({text, url});
        }
    } catch (std::out_of_range& e) {
        // Impossible to find the requested archive (bug ?)
        // We could propose a suggestion to do multi-zim search with:
        // url.setHost("library.search");
        // but we don't have a correct UI to select on which zim search, how to display results, ...
        // So do nothing for now
    }
    emit(searchFinished(suggestionList, m_token));
}
