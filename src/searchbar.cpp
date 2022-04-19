#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>
#include <QCursor>

#include "kiwixapp.h"
#include "suggestionlistworker.h"



void SearchBar::set_searchMode(bool searchMode)
{
    m_searchMode = searchMode;
    if (m_searchMode) {
        m_action->setIcon(QIcon(":/icons/search.svg"));
    } else {
        auto kiwixApp = KiwixApp::instance();
        if (kiwixApp->isCurrentArticleBookmarked()) {
            m_action->setIcon(QIcon(":/icons/reading-list-active.svg"));
        } else {
            m_action->setIcon(QIcon(":/icons/reading-list.svg"));
        }
    }
}

void SearchBar::on_action_triggered()
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
    m_action(new QAction(this))
{
    mp_typingTimer = new QTimer(this);
    mp_typingTimer->setSingleShot(true);
    setPlaceholderText(gt("search"));
    setToolTip(gt("search"));
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completer.setCaseSensitivity(Qt::CaseInsensitive);
    m_completer.setMaxVisibleItems(16);
    setCompleter(&m_completer);

    connect(m_action, &QAction::triggered,
            this, &SearchBar::on_action_triggered);
    addAction(m_action, QLineEdit::LeadingPosition);
    set_searchMode(true);

    QFile styleFile(":/css/popup.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    m_completer.popup()->setStyleSheet(style);

    qRegisterMetaType<QVector<QUrl>>("QVector<QUrl>");
    connect(mp_typingTimer, &QTimer::timeout, this, &SearchBar::updateCompletion);

    connect(this, &QLineEdit::textEdited, this,
            [=](const QString &text) {
                m_searchbarInput = text;
                m_returnPressed = false;
                mp_typingTimer->start(100);
    });
    connect(this, &QLineEdit::textChanged, this,
            [=](const QString &text) {
                Q_UNUSED(text)
                if (m_returnPressed) {
                    this->setText(m_searchbarInput);
                }
    });
    connect(this, &QLineEdit::returnPressed, this, [=]() {
        m_returnPressed = true;
    });
}

void SearchBar::hideSuggestions()
{
    m_completer.popup()->hide();
}

void SearchBar::clearSuggestions()
{
    QStringList empty;
    m_completionModel.setStringList(empty);
    m_urlList.clear();
}

void SearchBar::on_currentTitleChanged(const QString& title)
{
    if (this->hasFocus()) {
        return;
    }
    if (!title.startsWith("zim://")) {
        setText(title);
    } else {
        setText("");
    }
    set_searchMode(title.isEmpty());
    m_title = title;
}

void SearchBar::focusInEvent( QFocusEvent* event)
{
    if (event->reason() == Qt::MouseFocusReason) {
        if (QApplication::widgetAt(QCursor::pos()) != this) {
            // drop focus, mouse clicked on one of inner widget (m_action)
            clearFocus();
            return;
        }
    }

    setReadOnly(false);
    if (event->reason() == Qt::MouseFocusReason && text() == m_title) {
        clear();
    }
    if (event->reason() == Qt::ActiveWindowFocusReason ||
        event->reason() == Qt::MouseFocusReason) {
        /* need to connect every time because QLineEdit::focusOutEvent does:
         * QObject::disconnect(d->control->completer(), 0, this, 0);
         */
        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
        this, QOverload<const QModelIndex &>::of(&SearchBar::openCompletion));
    }
    QLineEdit::focusInEvent(event);
    set_searchMode(true);
}

void SearchBar::focusOutEvent(QFocusEvent* event)
{
    setReadOnly(true);
    if (event->reason() == Qt::MouseFocusReason && text().isEmpty()) {
        set_searchMode(false);
        setText(m_title);
    }
    deselect();
    return QLineEdit::focusOutEvent(event);
}

void SearchBar::updateCompletion()
{
    mp_typingTimer->stop();
    clearSuggestions();
    WebView* current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current || current->url().isEmpty() || m_searchbarInput.isEmpty()) {
        hideSuggestions();
        return;
    }
    m_token++;
    auto suggestionWorker = new SuggestionListWorker(m_searchbarInput, m_token, this);
    connect(suggestionWorker, &SuggestionListWorker::searchFinished, this,
    [=] (const QStringList& suggestions, const QVector<QUrl>& urlList, int token) {
        if (token != m_token) {
            return;
        }
        m_urlList = urlList;
        if (m_returnPressed) {
            openCompletion(suggestions.first(), 0);
            return;
        }
        m_completionModel.setStringList(suggestions);
        m_completer.complete();
    });
    connect(suggestionWorker, &SuggestionListWorker::finished, suggestionWorker, &QObject::deleteLater);
    suggestionWorker->start();
}

void SearchBar::openCompletion(const QModelIndex &index)
{
    if (m_urlList.size() != 0) {
        openCompletion(index.data().toString(), index.row());
    }
}

void SearchBar::openCompletion(const QString& text, int index)
{
    QUrl url;
    if (this->text().compare(text, Qt::CaseInsensitive) == 0) {
        url = m_urlList.at(index);
    } else {
        url = m_urlList.last();
    }
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(url, false);});
}
