#include "searchbar.h"

#include <QCompleter>
#include <QTimer>

#include "kiwixapp.h"

SearchBar::SearchBar(QWidget *parent) :
    QLineEdit(parent),
    m_completer(&m_completionModel, this),
    m_icon(":icons/search.svg")
{
    setTextMargins(37, 1, 1, 1);
    setPlaceholderText("Search");
    setClearButtonEnabled(true);
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
}

void SearchBar::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    QPainter painter(this);
    QPixmap pxm = m_icon.pixmap(height() - 6, height() - 6);
    painter.drawPixmap(3, 3, pxm);
}

void SearchBar::updateCompletion(const QString &text)
{
    QStringList wordList;
    m_urlList.clear();
    auto tabWidget = KiwixApp::instance()->getTabWidget();
    auto qurl = tabWidget->currentWidget()->url();
    m_currentHost = qurl.host();
    auto reader = KiwixApp::instance()->getLibrary()->getReader(m_currentHost);
    if ( reader == nullptr) {
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
    qurl.setHost(qurl.host());
    qurl.setPath("/" + QString::fromStdString(path));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
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

