#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>

#include "kiwixapp.h"
#include "suggestionlistworker.h"

BookmarkButton::BookmarkButton(QWidget *parent) :
    QToolButton(parent)
{
    connect(this, &QToolButton::triggered, this, &BookmarkButton::on_buttonClicked);
    connect(this, &QToolButton::triggered, this, &BookmarkButton::update_display);
    setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::Actions::ToggleAddBookmarkAction));

    auto library = KiwixApp::instance()->getLibrary();
    connect(library, &Library::bookmarksChanged, this, &BookmarkButton::update_display);
}

void BookmarkButton::update_display()
{
    auto isBookMarked = KiwixApp::instance()->isCurrentArticleBookmarked();
    auto buttonText = isBookMarked ? gt("remove-bookmark") : gt("add-bookmark");
    defaultAction()->setChecked(isBookMarked);
    defaultAction()->setToolTip(buttonText);
    defaultAction()->setText(buttonText);
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
    m_completer(&m_suggestionModel, this)
{
    setAlignment(KiwixApp::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);
    mp_typingTimer = new QTimer(this);
    mp_typingTimer->setSingleShot(true);

    /* Placeholder does not affect line edit alignment and is aligned to line
       edit purely by text direction (LTR leading RTL ending). Thus, we need
       directional mask to make it LTR at leading position.
       https://stackoverflow.com/questions/66430215/english-and-arabic-mixed-string-not-ordered-correctly-qt
    */
    const QString ltrConversionChar = QString{"\u200e"};
    setPlaceholderText(ltrConversionChar + gt("search"));
    setToolTip(gt("search"));
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completer.setCaseSensitivity(Qt::CaseInsensitive);
    m_completer.setMaxVisibleItems(16);
    setCompleter(&m_completer);

    m_completer.popup()->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/popup.css"));

    qRegisterMetaType<QList<SuggestionData>>("QList<SuggestionData>");
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

                /* Empty text is LTR which changes the line edit alignment.
                   Need to explicitly align right. This is a generalized
                   solution that aligns text to the direction of the app. */
                bool isSameDirection = text.isRightToLeft() == KiwixApp::isRightToLeft();
                setAlignment(isSameDirection ? Qt::AlignLeft : Qt::AlignRight);
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
    m_suggestionModel.resetSuggestions();
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
    [=] (const QList<SuggestionData>& suggestionList, int token) {
        if (token != m_token) {
            return;
        }

        m_suggestionModel.append(suggestionList);
        if (m_returnPressed) {
            openCompletion(suggestionList.first().text, 0);
            return;
        }
        m_completer.complete();
    });
    connect(suggestionWorker, &SuggestionListWorker::finished, suggestionWorker, &QObject::deleteLater);
    suggestionWorker->start();
}

void SearchBarLineEdit::openCompletion(const QModelIndex &index)
{
    if (m_suggestionModel.rowCount() != 0) {
        openCompletion(index.data().toString(), index.row());
    }
}

void SearchBarLineEdit::openCompletion(const QString& text, int index)
{
    QUrl url;
    if (this->text().compare(text, Qt::CaseInsensitive) == 0) {
        url = m_suggestionModel.index(index).data(Qt::UserRole).toUrl();
    } else {
        url = m_suggestionModel.index(m_suggestionModel.rowCount() - 1).data(Qt::UserRole).toUrl();
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
