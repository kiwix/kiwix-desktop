#include "searchbar.h"

#include <QCompleter>
#include <QFocusEvent>
#include <QScrollBar>
#include <QStyledItemDelegate>

#include "kiwixapp.h"
#include "suggestionlistworker.h"
#include "css_constants.h"
#include "suggestionlistdelegate.h"

namespace HeaderSectionCSS = CSS::PopupCSS::QHeaderView::section;

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
    const QString ltrConversionChar(u8"\u200e");
    setPlaceholderText(ltrConversionChar + gt("search"));
    setToolTip(gt("search"));
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completer.setCaseSensitivity(Qt::CaseInsensitive);

    /* The items should be less than fetch size to enable scrolling. */
    m_completer.setMaxVisibleItems(SuggestionListWorker::getFetchSize() / 2);
    m_completer.setWidget(this);

    /* QCompleter's uses default list views, which do not have headers. */
    m_completer.setPopup(m_suggestionView);

    /* The Delegate was overwritten by setPopup(), which is not style-aware */
    m_suggestionView->setItemDelegate(new SuggestionListDelegate(this));
    m_suggestionView->header()->setStretchLastSection(true);
    m_suggestionView->setRootIsDecorated(false);
    m_suggestionView->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/popup.css"));

    /* Configure the view to handle key events properly */
    m_suggestionView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suggestionView->setFocusPolicy(Qt::StrongFocus);
    m_suggestionView->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    /* Prevent wrapping navigation at list boundaries */
    m_suggestionView->setTabKeyNavigation(false);
    m_suggestionView->setProperty("_q_KeyboardScrolling", false);
    m_suggestionView->setAutoScroll(false);
    
    m_suggestionView->installEventFilter(this);

    const int contentHeight = HeaderSectionCSS::lineHeight;
    m_suggestionView->setIconSize(QSize(contentHeight, contentHeight));

    /* The suggestionView sizing unfortunately is not aware of headers. We
       have to do this manually. We also sized header the same as items.
    */
    connect(&m_suggestionModel, &QAbstractListModel::modelReset, [=](){
        /* +1 for header. */
        const int maxItem = m_completer.maxVisibleItems();
        const int count = std::min(m_suggestionModel.rowCount(), maxItem) + 1;

        const int itemHeight = m_suggestionView->sizeHintForRow(0);

        /* Extra space styling above header and below last suggestion item. */
        const int extraMargin = 2 * HeaderSectionCSS::marginTop;
        m_suggestionView->setFixedHeight(itemHeight * count + extraMargin);
    });

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

        /* Open default index when nothing is selected.

           Key_Return can be pressed during typing, where suggestions no longer
           match the text typed. Hence the suggestionsValid check.
         */
        const bool suggestionsValid = m_suggestionsAreValidFor == m_searchbarInput;
        if (!m_suggestionView->currentIndex().isValid() && suggestionsValid)
            openCompletion(getDefaulSuggestionIndex());
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

bool SearchBarLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    // Handle events for the suggestion view
    if (watched == m_suggestionView)
    {
        // Handle key press events
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            const auto key = keyEvent->key();
            const bool noModifiers = keyEvent->modifiers() == Qt::NoModifier;
            
            if (noModifiers)
            {
                auto selectionModel = m_suggestionView->selectionModel();
                auto currentIndex = selectionModel->currentIndex();
                int lastRow = m_suggestionModel.rowCount() - 1;
                
                // Handle down key at the last item
                if ((key == Qt::Key_Down || key == Qt::Key_PageDown) && currentIndex.row() == lastRow)
                {
                    // If more suggestions are available, fetch them
                    if (m_moreSuggestionsAreAvailable)
                    {
                        fetchMoreSuggestions();
                    }
                    return true; // Consume the event
                }
                
                // Handle up key at the first item
                if ((key == Qt::Key_Up || key == Qt::Key_PageUp) && currentIndex.row() == 0)
                {
                    return true; 
                }
                
                // Handle fetching more suggestions when near the end
                if ((key == Qt::Key_Down || key == Qt::Key_PageDown) && 
                    m_moreSuggestionsAreAvailable && 
                    currentIndex.row() >= lastRow - 2)
                {
                    // Let default behavior happen
                    bool result = QLineEdit::eventFilter(watched, event);
                    
                    // Then fetch more suggestions
                    fetchMoreSuggestions();
                    
                    // Ensure the new selection is visible
                    QTimer::singleShot(0, [this]() {
                        auto newIndex = m_suggestionView->selectionModel()->currentIndex();
                        if (newIndex.isValid()) {
                            m_suggestionView->scrollTo(newIndex, QAbstractItemView::EnsureVisible);
                        }
                    });
                    
                    return result;
                }
                
                // For regular navigation, allow default behavior then ensure selection is visible
                if (key == Qt::Key_Down || key == Qt::Key_Up || 
                    key == Qt::Key_PageDown || key == Qt::Key_PageUp)
                {
                    bool result = QLineEdit::eventFilter(watched, event);
                    
                    // Ensure selection is visible
                    QTimer::singleShot(0, [this]() {
                        auto newIndex = m_suggestionView->selectionModel()->currentIndex();
                        if (newIndex.isValid()) {
                            m_suggestionView->scrollTo(newIndex, QAbstractItemView::EnsureVisible);
                        }
                    });
                    
                    return result;
                }
            }
        }
        
        // Post-key processing to maintain proper selection
        if (event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            const auto key = keyEvent->key();
            
            if (key == Qt::Key_Down || key == Qt::Key_PageDown || key == Qt::Key_Up || key == Qt::Key_PageUp)
            {
                auto selectionModel = m_suggestionView->selectionModel();
                
                // If no selection exists after key navigation, select the first/last item
                if (!selectionModel->currentIndex().isValid())
                {
                    int row = (key == Qt::Key_Down || key == Qt::Key_PageDown) ? 0 : m_suggestionModel.rowCount() - 1;
                    auto index = m_suggestionModel.index(row);
                    if (index.isValid())
                    {
                        selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                        m_suggestionView->scrollTo(index, QAbstractItemView::EnsureVisible);
                    }
                    return true;
                }
            }
        }
    }
    
    return QLineEdit::eventFilter(watched, event);
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
        event->reason() == Qt::ShortcutFocusReason ||
        event->reason() == Qt::PopupFocusReason) {
        connect(&m_completer, QOverload<const QString &>::of(&QCompleter::activated),
                this, &QLineEdit::setText,Qt::UniqueConnection);

        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
                this, QOverload<const QModelIndex &>::of(&SearchBarLineEdit::openCompletion),
                Qt::UniqueConnection);

        connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::highlighted), this,
                [=](const QModelIndex &index){
                    setText(index.isValid() ? index.data().toString() : m_searchbarInput);
                },
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
    disconnect(&m_completer, nullptr, this, nullptr);
    return QLineEdit::focusOutEvent(event);
}

void SearchBarLineEdit::updateCompletion()
{
    mp_typingTimer->stop();
    clearSuggestions();
    const auto& multiZim = KiwixApp::instance()->getSearchBar().getMultiZimButton();
    if (multiZim.getZimIds().isEmpty() || m_searchbarInput.isEmpty()) {
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

    const auto suggestionScroller = m_suggestionView->verticalScrollBar();
    const auto scrollMin = suggestionScroller->minimum();
    const auto scrollMax = suggestionScroller->maximum();
    const bool scrolledToEnd = value == suggestionScroller->maximum();

    if (m_aboutToScrollPastEnd)
    {
        if (scrolledToEnd)
        {
            /* Store current scroll position before fetching more */
            const int currentScrollPos = suggestionScroller->value();
            
            /* Fetch more suggestions */
            fetchMoreSuggestions();
            
            /* Restore scroll position after a short delay to ensure the view has updated */
            QTimer::singleShot(0, [=]() {
                suggestionScroller->setValue(currentScrollPos);
            });
            
            m_aboutToScrollPastEnd = false;
        }
        else
        {
            /* Scrolling past end did not happen - remove the extra scroll room */
            suggestionScroller->setRange(scrollMin, scrollMax - 1);
            m_aboutToScrollPastEnd = false;
        }
    }
    else if (scrolledToEnd)
    {
        /* The user has scrolled to end - monitor for further scrolling */
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
        const auto app = KiwixApp::instance();
        const bool newTab = app->getTabWidget()->currentWebView() == nullptr;
        QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(url, newTab);});
    }
}

void SearchBarLineEdit::onInitialSuggestions(int)
{
    if (m_returnPressed) {
        openCompletion(getDefaulSuggestionIndex());
    } else {
        m_completer.complete(getCompleterRect());

        /* Select nothing by default */
        const auto completerSelModel = m_suggestionView->selectionModel();
        completerSelModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current);
    }
}

void SearchBarLineEdit::onAdditionalSuggestions(int start)
{
    // Get the model index for the first new suggestion
    const auto completerStartIdx = m_suggestionModel.index(start, 0);
    
    // If we were at the last item before fetching more suggestions,
    // select the first new suggestion (at index 'start')
    auto selectionModel = m_suggestionView->selectionModel();
    auto currentIndex = selectionModel->currentIndex();
    
    /* Block signals temporarily to prevent unwanted scroll events */
    const bool oldState = m_suggestionView->blockSignals(true);
    
    // If there was no selection or we were at the end, select the first new item
    if (!currentIndex.isValid() || currentIndex.row() == start - 1)
    {
        selectionModel->setCurrentIndex(completerStartIdx, 
                                       QItemSelectionModel::ClearAndSelect | 
                                       QItemSelectionModel::Rows);
    }
    
    // Ensure the selected item is visible
    m_suggestionView->scrollTo(selectionModel->currentIndex(), 
                              QAbstractItemView::PositionAtCenter);
    
    /* Restore signal state */
    m_suggestionView->blockSignals(oldState);
    
    // Ensure the view is visible
    m_suggestionView->show();
}

void SearchBarLineEdit::fetchSuggestions(NewSuggestionHandlerFuncPtr callback)
{
    const int start = m_suggestionModel.countOfRegularSuggestions();
    const auto searchText = m_searchbarInput;
    const auto suggestionWorker = new SuggestionListWorker(searchText, m_token, start, this);
    connect(suggestionWorker, &SuggestionListWorker::searchFinished, this,
            [=] (const QList<SuggestionData>& suggestionList, int token) {
                if (token != m_token) {
                    return;
                }

                m_suggestionModel.append(suggestionList);
                m_suggestionsAreValidFor = searchText;
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

/* Line edit does not span the entire searchBar. Completer is displayed
   based on line edit, and thus shifting and resizing is needed.
*/
QRect SearchBarLineEdit::getCompleterRect() const
{
    auto& searchBar = KiwixApp::instance()->getSearchBar();
    const auto& searchGeo = searchBar.geometry();
    const auto& searchLineEditGeo = searchBar.getLineEdit().geometry();

    const int margin = CSS::SearchBar::margin;
    const int border = CSS::SearchBar::border;
    const int spaceAround = margin + border;

    /* Border and margin are not accounted in height and width. */
    const int top = searchGeo.height() - 2 * spaceAround;
    const int width = searchGeo.width() - 2 * spaceAround;

    /* Shift completer to one of the two laterals of search bar, where which
       one it shifted to dependes on whether the line edit is flipped.
    */
    int left = -searchLineEditGeo.left();

    /* When not flipped, left() is relative to within the search bar border,
       thus, we shift by spaceAround to match the side of search bar.

       When flipped, the completer starts at the right end of the search bar
       We shift it by width to make the completer start at left lateral of
       search bar. Since in a flipped state, left() also considered the opposite
       side's border, which means we need to shift by a border width in
       addition to spaceAround.
    */
    left += isRightToLeft() ? -width + spaceAround + border : spaceAround;

    /* Can't set height to 0. Will cause rectangle to be ignored. */
    return QRect(QPoint(left, top), QSize(width, 1));
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
            &MultiZimButton::updateDisplay);
    connect(this, &SearchBar::currentTitleChanged, &m_multiZimButton,
            &MultiZimButton::updateDisplay);
}
