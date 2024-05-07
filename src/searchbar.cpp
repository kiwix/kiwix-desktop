#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>

#include "kiwixapp.h"
#include "suggestionlistworker.h"

BookmarkButton::BookmarkButton(QWidget *parent) :
    QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, &BookmarkButton::on_buttonClicked);
    connect(this, &QPushButton::clicked, this, &BookmarkButton::update_display);
}

void BookmarkButton::update_display()
{
    auto kiwixApp = KiwixApp::instance();
    if (kiwixApp->isCurrentArticleBookmarked()) {
        setIcon(QIcon(":/icons/star-active.svg"));
        setToolTip(gt("remove-bookmark"));
    } else {
        setIcon(QIcon(":/icons/star.svg"));
        setToolTip(gt("add-bookmark"));
    }
    setIconSize(QSize(32, 32));
}

void BookmarkButton::on_buttonClicked()
{
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
    library->save();
}

SearchBarLineEdit::SearchBarLineEdit(QWidget *parent) :
    QLineEdit(parent),
    m_completer(&m_completionModel, this)
{
    mp_typingTimer = new QTimer(this);
    mp_typingTimer->setSingleShot(true);
    setPlaceholderText(gt("search"));
    setToolTip(gt("search"));
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completer.setCaseSensitivity(Qt::CaseInsensitive);
    m_completer.setMaxVisibleItems(16);
    setCompleter(&m_completer);

    m_completer.popup()->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/popup.css"));

    qRegisterMetaType<QVector<QUrl>>("QVector<QUrl>");
    connect(mp_typingTimer, &QTimer::timeout, this, &SearchBarLineEdit::updateCompletion);

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
    
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::SearchArticleAction), &QAction::triggered,
            this, [=]() {
                this->selectAll();
                this->setFocus(Qt::ShortcutFocusReason);
            });
}

void SearchBarLineEdit::hideSuggestions()
{
    m_completer.popup()->hide();
}

void SearchBarLineEdit::clearSuggestions()
{
    QStringList empty;
    m_completionModel.setStringList(empty);
    m_urlList.clear();
}

void SearchBarLineEdit::on_currentTitleChanged(const QString& title)
{
    if (this->hasFocus()) {
        return;
    }
    if (!title.startsWith("zim://")) {
        setText(title);
    } else {
        setText("");
    }
    m_title = title;
}

void SearchBarLineEdit::focusInEvent( QFocusEvent* event)
{
    setReadOnly(false);
    if (event->reason() == Qt::MouseFocusReason && text() == m_title) {
        clear();
    }
    if (event->reason() == Qt::ActiveWindowFocusReason ||
        event->reason() == Qt::MouseFocusReason ||
        event->reason() == Qt::ShortcutFocusReason) {
        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
        this, QOverload<const QModelIndex &>::of(&SearchBarLineEdit::openCompletion));
    }
    QLineEdit::focusInEvent(event);
}

void SearchBarLineEdit::focusOutEvent(QFocusEvent* event)
{
    setReadOnly(true);
    if (event->reason() == Qt::MouseFocusReason && text().isEmpty()) {
        setText(m_title);
    }
    deselect();
    return QLineEdit::focusOutEvent(event);
}

void SearchBarLineEdit::updateCompletion()
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

void SearchBarLineEdit::openCompletion(const QModelIndex &index)
{
    if (m_urlList.size() != 0) {
        openCompletion(index.data().toString(), index.row());
    }
}

void SearchBarLineEdit::openCompletion(const QString& text, int index)
{
    QUrl url;
    if (this->text().compare(text, Qt::CaseInsensitive) == 0) {
        url = m_urlList.at(index);
    } else {
        url = m_urlList.last();
    }
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(url, false);});
}

SearchBar::SearchBar(QWidget *parent) :
    QToolBar(parent),
    m_searchBarLineEdit(this),
    m_bookmarkButton(this)
{
    QLabel* searchIconLabel = new QLabel; 
    searchIconLabel->setObjectName("searchIcon");
    searchIconLabel->setPixmap(QIcon(":/icons/search.svg").pixmap(QSize(27, 27)));

    setIconSize(QSize(32, 32));

    addWidget(searchIconLabel);
    addWidget(&m_searchBarLineEdit);
    addWidget(&m_bookmarkButton);

    connect(this, &SearchBar::currentTitleChanged, &m_searchBarLineEdit,
            &SearchBarLineEdit::on_currentTitleChanged);
    connect(this, &SearchBar::currentTitleChanged, &m_bookmarkButton,
            &BookmarkButton::update_display);
}
