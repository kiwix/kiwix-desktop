#include "searchbar.h"

#include <QCompleter>
#include <QTimer>
#include <QFocusEvent>

#include "kiwixapp.h"

SearchButton::SearchButton(QWidget *parent) :
    QPushButton(parent),
    m_searchMode(true)
{
    setFlat(true);
    setIcon(QIcon(":/icons/search.svg"));
    connect(this, &QPushButton::clicked, this, &SearchButton::on_buttonClicked);
}

void SearchButton::set_searchMode(bool searchMode)
{
    m_searchMode = searchMode;
    if (m_searchMode) {
        setIcon(QIcon(":/icons/search.svg"));
    } else {
        auto kiwixApp = KiwixApp::instance();
        if (kiwixApp->isCurrentArticleBookmarked()) {
            setIcon(QIcon(":/icons/reading-list-active.svg"));
        } else {
            setIcon(QIcon(":/icons/reading-list.svg"));
        }
    }
}

void SearchButton::on_buttonClicked()
{
    if (m_searchMode)
        return;

    auto kiwixApp = KiwixApp::instance();
    auto library = kiwixApp->getLibrary();
    auto tabWidget = kiwixApp->getTabWidget();
    if (kiwixApp->isCurrentArticleBookmarked()) {
        auto zimid = tabWidget->currentZimId();
        library->removeBookmark(
            zimid, tabWidget->currentArticleUrl()
        );
    } else {
        kiwix::Bookmark bookmark;
        auto zimid = tabWidget->currentZimId().toStdString();
        bookmark.setBookId(zimid);
        bookmark.setUrl(tabWidget->currentArticleUrl().toStdString());
        bookmark.setTitle(tabWidget->currentArticleTitle().toStdString());
        library->addBookmark(bookmark);
    }
    set_searchMode(false);
    library->save();
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
    connect(KiwixApp::instance(), &KiwixApp::currentTitleChanged,
            this, &SearchBar::on_currentTitleChanged);
}

void SearchBar::on_currentTitleChanged(const QString& title)
{
    setText(title);
    m_button.set_searchMode(title.isEmpty());
}

void SearchBar::focusInEvent( QFocusEvent* event)
{
    if (event->reason() == Qt::MouseFocusReason)
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
    m_currentZimId = qurl.host();
    m_currentZimId.resize(m_currentZimId.length()-4);
    auto reader = KiwixApp::instance()->getLibrary()->getReader(m_currentZimId);
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
    qurl.setHost(zimId+".zim");
    qurl.setPath("/" + QString::fromStdString(path));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, false);});
    clearFocus();
}

void SearchBar::openCompletion(const QModelIndex &index)
{
    auto url = m_urlList.at(index.row());
    QUrl qurl;
    qurl.setScheme("zim");
    qurl.setHost(m_currentZimId+".zim");
    qurl.setPath(QString::fromStdString(url));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
}

