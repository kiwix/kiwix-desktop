#include "searchbar.h"

#include <QCompleter>
#include <QTimer>

#include "kiwixapp.h"

SearchButton::SearchButton(QWidget *parent) :
    QPushButton(parent),
    m_searchMode(true)
{
    setFlat(true);
    setIcon(QIcon(":/icons/search.svg"));
}

void SearchButton::set_searchMode(bool searchMode)
{
    if (searchMode == m_searchMode)
        return;

    m_searchMode = searchMode;
    if (m_searchMode) {
        setIcon(QIcon(":/icons/search.svg"));
    } else {
        setIcon(QIcon(":/icons/reading-list.svg"));
    }
}

SearchBar::SearchBar(QWidget *parent) :
    QLineEdit(parent),
    m_completer(&m_completionModel, this),
    m_button(this)
{
    setPlaceholderText(tr("Search"));
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setCompleter(&m_completer);
    connect(this, &QLineEdit::textEdited, this, &SearchBar::updateCompletion);
#if 0 //The `activated` signal seems to not be emitted if user navigate in the page.
      // Something is broken here .. :/
    connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, &SearchBar::openCompletion);
#else
    connect(this, &QLineEdit::returnPressed, this, &SearchBar::openTitle);
#endif
    connect(KiwixApp::instance(), &KiwixApp::currentTitleChanged, this,
            [=](const QString& title) {
        setText(title);
        m_button.set_searchMode(false);
    });
}


void SearchBar::focusInEvent( QFocusEvent* event)
{
    clear();
    QLineEdit::focusInEvent(event);
    m_button.set_searchMode(true);
}

void SearchBar::updateCompletion(const QString &text)
{
    QStringList wordList;
    m_urlList.clear();
    auto currentWidget = KiwixApp::instance()->getTabWidget()->currentWidget();
    if (!currentWidget) {
        m_completionModel.setStringList(wordList);
        return;
    }
    auto qurl = currentWidget->url();
    m_currentHost = qurl.host();
    auto reader = KiwixApp::instance()->getLibrary()->getReader(m_currentHost);
    if (!reader) {
        m_completionModel.setStringList(wordList);
        return;
    }
    reader->searchSuggestionsSmart(text.toStdString(), 15);
    std::string title, url;
    while (reader->getNextSuggestion(title, url)) {
        wordList << QString::fromStdString(title);
        m_urlList.push_back(url);
    }
    m_completionModel.setStringList(wordList);
}

void SearchBar::openTitle()
{
    QString title = text();
    clear();
    auto tabWidget = KiwixApp::instance()->getTabWidget();
    auto zimId = tabWidget->currentZimId();
    auto reader = KiwixApp::instance()->getLibrary()->getReader(zimId);
    if ( reader == nullptr) {
        return;
    }
    std::string path;
    try {
        auto entry = reader->getEntryFromTitle(title.toStdString());
        path = entry.getPath();
    } catch (kiwix::NoEntry& e)
    {
        return;
    }

    QUrl qurl;
    qurl.setScheme("zim");
    qurl.setHost(zimId);
    qurl.setPath("/" + QString::fromStdString(path));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
    clearFocus();
}

void SearchBar::openCompletion(const QModelIndex &index)
{
    auto url = m_urlList.at(index.row());
    QUrl qurl;
    qurl.setScheme("zim");
    qurl.setHost(m_currentHost);
    qurl.setPath(QString::fromStdString(url));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
}

