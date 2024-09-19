#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>
#include <QScrollBar>

#include "kiwixapp.h"
#include "suggestionlistworker.h"
#include "suggestionlistdelegate.h"

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
    m_completer.setMaxVisibleItems(8);
    m_completer.setWidget(this);

    /* QCompleter's uses default list views, which do not have headers. */
    m_completer.setPopup(m_suggestionView);

    m_suggestionView->setItemDelegate(new SuggestionListDelegate(this));
    m_suggestionView->header()->setStretchLastSection(true);
    m_suggestionView->setRootIsDecorated(false);
    m_suggestionView->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/popup.css"));

    /* See line-height&padding resources/css/popup.css QHeaderView::section. */
    m_suggestionView->setIconSize(QSize(24, 24));
    connect(&m_suggestionModel, &QAbstractListModel::modelReset, [=](){
        /* +1 for header. +10px for 5px top&bottom extra space */
        int count = std::min(m_suggestionModel.rowCount(), m_completer.maxVisibleItems());
        m_suggestionView->setMinimumHeight(m_suggestionView->sizeHintForRow(0) * (count + 1) + 10);
    });

    connect(m_suggestionView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SearchBarLineEdit::onScrollToEnd);

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
    if (auto e = static_cast<QKeyEvent *>(event))
    {
        if (e->key() == Qt::Key_PageDown && m_scrolledEndBefore)
        {
            m_scrolledEndBefore = false;
            fetchMoreSuggestion();
            return true;
        }
    }
    return false;
}

void SearchBarLineEdit::clearSuggestions()
{
    m_suggestionModel.resetSuggestions();
    m_suggestionModel.resetUrlList();
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
        event->reason() == Qt::ShortcutFocusReason ||
        event->reason() == Qt::PopupFocusReason) {
        connect(&m_completer, QOverload<const QString &>::of(&QCompleter::activated),
        this, &QLineEdit::setText,Qt::UniqueConnection);

        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
        this, QOverload<const QModelIndex &>::of(&SearchBarLineEdit::openCompletion),
        Qt::UniqueConnection);

        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
        this, [=](const QModelIndex &index){
            setText(index.isValid() ? index.data().toString() : m_searchbarInput);
        }, Qt::UniqueConnection);
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
    disconnect(&m_completer, nullptr, this, nullptr);
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
    auto suggestionWorker = new SuggestionListWorker(m_searchbarInput, m_token, 0, this);
    connect(suggestionWorker, &SuggestionListWorker::searchFinished, this,
    [=] (const QStringList& suggestions, const QVector<QUrl>& urlList, bool hasFullText, int token) {
        if (token != m_token) {
            return;
        }
        m_suggestionModel.resetUrlList(urlList);
        if (m_returnPressed) {
            openCompletion(m_suggestionModel.index(0));
            return;
        }

        m_suggestionModel.setHasFullText(hasFullText);
        m_suggestionModel.resetSuggestions(suggestions);
        m_completer.complete(getCompleterRect());

        /* Make row 0 appear but does not highlight it */
        m_suggestionView->selectionModel()->setCurrentIndex(m_suggestionModel.index(0), QItemSelectionModel::Current);
    });
    connect(suggestionWorker, &SuggestionListWorker::finished, suggestionWorker, &QObject::deleteLater);
    suggestionWorker->start();
}

QRect SearchBarLineEdit::getCompleterRect()
{
    auto& searchBar = KiwixApp::instance()->getSearchBar();
    auto searchGeo = searchBar.geometry();
    auto searchLineEditGeo = searchBar.getLineEdit().geometry();

    /* See SearchBar border and margin size in resources/css/style.css */
    int top = searchGeo.height() - 6; /* top&bottom margin + border */
    int width = searchGeo.width() - 6; /* left&right margin + border */
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool lineEditFlipped = KiwixApp::isRightToLeft();
#else
    bool lineEditFlipped = m_searchbarInput.isRightToLeft();
#endif

    /* When not flipped, match left of completer to search bar. Popups are
       shifted by margin and border so we undo those changes to stay in border.

       When flipped, we first match left of completer to right of search bar.
       In addition to the popup shift due to border and margin, We move right by
       one more border size to match the right border pixel. We then move it 
       left by width to match left of search bar.    
    */
    int left = lineEditFlipped ? -searchLineEditGeo.left() + 4 - width
                               : -searchLineEditGeo.left() + 3;

    /* Can't set height to 0. Will cause rectangle to be ignored. */
    return QRect(QPoint(left, top), QSize(width, 1));
}

void SearchBarLineEdit::fetchMoreSuggestion()
{
    int start = m_suggestionModel.fetchEndIndex().row() + 1;
    auto suggestionWorker = new SuggestionListWorker(m_searchbarInput, m_token, start, this);
    connect(suggestionWorker, &SuggestionListWorker::searchFinished, this,
    [=] (const QStringList& suggestions, const QVector<QUrl>& urlList, bool, int token) {
        if (token != m_token) {
            return;
        }

        m_suggestionModel.append(suggestions, urlList);

        /* Set selection to be at the last row of the previous list.

           m_completer has a private QCompleterModel wrapper class for 
           m_completer.popup(). Page Down behavior only works if indexes' parent
           is that instance.
        */
        m_suggestionView->setCurrentIndex(m_suggestionView->model()->index(start, 0));
        m_suggestionView->show();

        
    });
    connect(suggestionWorker, &SuggestionListWorker::finished, suggestionWorker, &QObject::deleteLater);
    suggestionWorker->start();
}

void SearchBarLineEdit::onScrollToEnd(int value)
{
    if (m_suggestionModel.noMoreSuggestion())
    {
        m_scrolledEndBefore = false;
        return;
    }

    if (!m_completer.popup()->currentIndex().isValid())
    {
        m_scrolledEndBefore = false;

        /* m_completer.popup()->currentIndex() being invalid means the list
           has been scrolled from bottom to top. We undo this here, as it avoids
           scroll bar flicker as well. Block signal to avoid recursion.
        */
        auto old = m_suggestionView->verticalScrollBar()->blockSignals(true);
        m_suggestionView->scrollToBottom();
        m_suggestionView->verticalScrollBar()->blockSignals(old);
        
        return fetchMoreSuggestion();
    }

    auto suggestionScroller = m_suggestionView->verticalScrollBar();
    bool scrolledToEnd = value == suggestionScroller->maximum();

    /* We only fetch when user scrolls down twice, otherwise user can never
        reach the fulltext option. Create this illusion there are more items
        by manually extending scroller.
    */
    auto scrollMin = suggestionScroller->minimum();
    auto scrollMax = suggestionScroller->maximum();
    if (m_scrolledEndBefore && scrolledToEnd)
        fetchMoreSuggestion();
    else if (scrolledToEnd)
        suggestionScroller->setRange(scrollMin, scrollMax + 1);
    else if (m_scrolledEndBefore)
        suggestionScroller->setRange(scrollMin, scrollMax - 1);

    m_scrolledEndBefore = !m_scrolledEndBefore && scrolledToEnd;
}

void SearchBarLineEdit::openCompletion(const QModelIndex &index)
{
    if (m_suggestionModel.rowCount() != 0) 
    {
        QUrl url;
        auto editText = index.data().toString();
        if (this->text().compare(editText, Qt::CaseInsensitive) == 0) {
            url = index.data(Qt::UserRole).toUrl();
        } else {
            url = m_suggestionModel.lastIndex().data(Qt::UserRole).toUrl();
        }
        QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(url, false);});
    }
}

SearchBar::SearchBar(QWidget *parent) :
    QToolBar(parent),
    m_searchBarLineEdit(this),
    m_bookmarkButton(this),
    m_multiZimButton(this)
{
    QLabel* searchIconLabel = new QLabel; 
    searchIconLabel->setObjectName("searchIcon");
    searchIconLabel->setPixmap(QIcon(":/icons/search.svg").pixmap(QSize(27, 27)));

    setIconSize(QSize(32, 32));

    addWidget(searchIconLabel);
    addWidget(&m_searchBarLineEdit);
    addWidget(&m_bookmarkButton);
    addWidget(&m_multiZimButton);

    connect(this, &SearchBar::currentTitleChanged, &m_searchBarLineEdit,
            &SearchBarLineEdit::on_currentTitleChanged);
    connect(this, &SearchBar::currentTitleChanged, &m_bookmarkButton,
            &BookmarkButton::update_display);
    connect(KiwixApp::instance()->getContentManager(),
            &ContentManager::booksChanged, &m_multiZimButton,
            &MultiZimButton::update_display);
    connect(this, &SearchBar::currentTitleChanged, &m_multiZimButton,
            &MultiZimButton::update_display);
}
