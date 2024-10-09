#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>
#include <QScrollBar>

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
    m_suggestionView(new QTreeView),
    m_completer(&m_suggestionModel, this)
{
    installEventFilter(this);
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

    /* The items should be less than fetch size to enable scrolling. */
    m_completer.setMaxVisibleItems(SuggestionListWorker::getFetchSize() / 2);
    setCompleter(&m_completer);

    /* QCompleter's uses default list views, which do not have headers. */
    m_completer.setPopup(m_suggestionView);

    m_suggestionView->header()->setStretchLastSection(true);
    m_suggestionView->setRootIsDecorated(false);
    m_suggestionView->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/popup.css"));

    connect(m_suggestionView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SearchBarLineEdit::onScroll);

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
    m_suggestionView->hide();
}

bool SearchBarLineEdit::eventFilter(QObject *, QEvent *event)
{
    if (!(m_aboutToScrollPastEnd && m_moreSuggestionsAreAvailable))
        return false;

    if (const auto e = dynamic_cast<QKeyEvent *>(event))
    {
        const auto key = e->key();
        const bool isScrollDownKey = key == Qt::Key_Down || key == Qt::Key_PageDown;
        const bool noModifiers = e->modifiers().testFlag(Qt::NoModifier);
        
        if (isScrollDownKey && noModifiers)
        {
            m_aboutToScrollPastEnd = false;
            fetchMoreSuggestions();
            return true;
        }
    }
    return false;
}

void SearchBarLineEdit::clearSuggestions()
{
    m_suggestionModel.resetSuggestions();
    m_moreSuggestionsAreAvailable = false;
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
                this, QOverload<const QModelIndex &>::of(&SearchBarLineEdit::openCompletion),
                Qt::UniqueConnection);
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
    fetchSuggestions(&SearchBarLineEdit::onInitialSuggestions);
}

void SearchBarLineEdit::fetchMoreSuggestions()
{
    /* TODO: Refactor suggestion worker to re-use zim::SuggestionSearcher for
       fetching more suggestion in a single archive. Currently we create a 
       searcher for every fetch, and discarded after one use.
    */
    fetchSuggestions(&SearchBarLineEdit::onAdditionalSuggestions);
}

void SearchBarLineEdit::onScroll(int value)
{
    if (!m_moreSuggestionsAreAvailable)
    {
        m_aboutToScrollPastEnd = false;
        return;
    }

    /* Scrolling using key_down past end will teleport scroller to the top.
       We undo this here. Block signal to avoid recursion. We cannot find a way
       to intercept the scrolling in eventFilter so, until we find out how, this
       code is here to stay.
    */
    if (!m_suggestionView->currentIndex().isValid())
    {
        const auto old = m_suggestionView->verticalScrollBar()->blockSignals(true);
        m_suggestionView->scrollToBottom();
        m_suggestionView->verticalScrollBar()->blockSignals(old);
        return;
    }

    const auto suggestionScroller = m_suggestionView->verticalScrollBar();
    const auto scrollMin = suggestionScroller->minimum();
    const auto scrollMax = suggestionScroller->maximum();
    const bool scrolledToEnd = value == suggestionScroller->maximum();
    if (m_aboutToScrollPastEnd)
    {
        if (scrolledToEnd)
        {
            /* The user's intention to scroll past end has been confirmed */
            fetchMoreSuggestions();
            m_aboutToScrollPastEnd = false; /* Relax until next time */
        }
        else
        {
            /* Scrolling past end did not happen - remove the extra scroll
               room created for detecting the intention of scrolling past end
            */
            suggestionScroller->setRange(scrollMin, scrollMax - 1);
            m_aboutToScrollPastEnd = false; /* ... and relax */
        }
    }
    else if (scrolledToEnd)
    {
        /* The user has scrolled to end - monitor for furthur scrolling */
        m_aboutToScrollPastEnd = true;
        /* Create some fictitious room for an extra scroll */
        suggestionScroller->setRange(scrollMin, scrollMax + 1);
    }
}

void SearchBarLineEdit::openCompletion(const QModelIndex &index)
{
    if (index.isValid())
    {
        const QUrl url = index.data(Qt::UserRole).toUrl();
        QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(url, false);});
    }
}

void SearchBarLineEdit::onInitialSuggestions(int)
{
    if (m_returnPressed) {
        openCompletion(getDefaulSuggestionIndex());
    } else {
        m_completer.complete();

        /* Make row 0 appear but do not highlight it */
        const auto completerFirstIdx = m_suggestionView->model()->index(0, 0);
        const auto completerSelModel = m_suggestionView->selectionModel();
        completerSelModel->setCurrentIndex(completerFirstIdx, QItemSelectionModel::Current);
    }
}

void SearchBarLineEdit::onAdditionalSuggestions(int start)
{
    /* Set selection to be at the last row of the previous list */
    const auto completerStartIdx = m_suggestionView->model()->index(start, 0);
    m_suggestionView->setCurrentIndex(completerStartIdx);
    m_suggestionView->show();
}

void SearchBarLineEdit::fetchSuggestions(NewSuggestionHandlerFuncPtr callback)
{
    const int start = m_suggestionModel.countOfRegularSuggestions();
    const auto suggestionWorker = new SuggestionListWorker(m_searchbarInput, m_token, start, this);
    connect(suggestionWorker, &SuggestionListWorker::searchFinished, this,
            [=] (const QList<SuggestionData>& suggestionList, int token) {
                if (token != m_token) {
                    return;
                }

                m_suggestionModel.append(suggestionList);
                const int listSize = suggestionList.size();
                const bool hasFullText = listSize > 0 && suggestionList.back().isFullTextSearchSuggestion();
                const int maxFetchSize = SuggestionListWorker::getFetchSize() + hasFullText;
                m_moreSuggestionsAreAvailable = listSize >= maxFetchSize;
                (this->*callback)(start);
            });
    connect(suggestionWorker, &SuggestionListWorker::finished, suggestionWorker, &QObject::deleteLater);
    suggestionWorker->start();
}

QModelIndex SearchBarLineEdit::getDefaulSuggestionIndex() const
{
    const auto firstSuggestionIndex = m_suggestionModel.index(0);
    if (!firstSuggestionIndex.isValid())
        return firstSuggestionIndex;

    /* If the first entry matches the typed text, use it as default, otherwise
       use the last entry if fulltext search exist. */
    const auto firstSuggestionText = firstSuggestionIndex.data().toString();
    if (this->text().compare(firstSuggestionText, Qt::CaseInsensitive) == 0)
        return firstSuggestionIndex;
    else if (m_suggestionModel.hasFullTextSearchSuggestion())
        return m_suggestionModel.index(m_suggestionModel.rowCount() - 1);
    return QModelIndex();
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
