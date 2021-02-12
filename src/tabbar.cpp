#include "tabbar.h"

#include "kiwixapp.h"
#include <QAction>
#include <QTimer>
#include <QWebEnginePage>
#include <QToolButton>
#include <QToolTip>
#include <QCursor>
#include <QPainter>

#define QUITIFNULL(VIEW) if (nullptr==(VIEW)) { return; }
#define QUITIFNOTCURRENT(VIEW) if((VIEW)!=currentWidget()) {return;}
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentWidget();}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent),
    m_settingsIndex(-1)
{
    QTabBar::setDrawBase(false);
    setTabsClosable(true);
    setElideMode(Qt::ElideNone);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    setIconSize(QSize(30, 30));
    connect(this, &QTabBar::currentChanged, this, &TabBar::onCurrentChanged);
    auto app = KiwixApp::instance();

    connect(app->getAction(KiwixApp::NewTabAction), &QAction::triggered,
            this, [=]() {
                this->createNewTab(true);
                auto topWidget = KiwixApp::instance()->getMainWindow()->getTopWidget();
                topWidget->getSearchBar().setFocus(Qt::MouseFocusReason);
                topWidget->getSearchBar().clear();
                topWidget->getSearchBar().clearSuggestions();
                topWidget->getSearchBar().hideSuggestions();
          });
    connect(app->getAction(KiwixApp::CloseTabAction), &QAction::triggered,
            this, [=]() {
                auto index = this->tabAt(mapFromGlobal(QCursor::pos()));
                if (index <= 0) {
                    return;
                }
                this->closeTab(index);
            });
    connect(app->getAction(KiwixApp::OpenHomePageAction), &QAction::triggered,
            this, [=]() {
                auto current = this->currentWebView();
                QUITIFNULL(current);
                current->setUrl("zim://" + current->zimId() + ".zim/");
            });
    connect(app->getAction(KiwixApp::SettingAction), &QAction::triggered,
            this, [=]() {
                if (KiwixApp::instance()->getSettingsManager()->isSettingsViewdisplayed()) {
                    setCurrentIndex(m_settingsIndex);
                    return;
                }
                int index = currentIndex() + 1;
                m_settingsIndex = index;
                SettingsManagerView* view = KiwixApp::instance()->getSettingsManager()->getView();
                mp_stackedWidget->insertWidget(index, view);
                insertTab(index,QIcon(":/icons/settings.svg"), gt("settings"));
                QToolButton *tb = new QToolButton(this);
                tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::CloseTabAction));
                setTabButton(index, QTabBar::RightSide, tb);
                setCurrentIndex(index);
            });
}

void TabBar::setStackedWidget(QStackedWidget *widget) {
    mp_stackedWidget = widget;
    connect(this, &QTabBar::currentChanged,
            widget, &QStackedWidget::setCurrentIndex);
}

void TabBar::setContentManagerView(ContentManagerView* view)
{
    qInfo() << "add widget";
    mp_contentManagerView = view;
    mp_stackedWidget->addWidget(mp_contentManagerView);
    mp_stackedWidget->show();
    addTab(QIcon(":/icons/library-icon.svg"), "");
    setTabButton(0, RightSide, nullptr);
}

void TabBar::setNewTabButton()
{
    QToolButton *tb = new QToolButton();
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::NewTabAction));
    tb->setIcon(QIcon(":/icons/new-tab-icon.svg"));
    addTab("");
    setTabEnabled(1, false);
    setTabButton(1, QTabBar::LeftSide, tb);
    tabButton(1, QTabBar::RightSide)->deleteLater();
    setTabButton(1, QTabBar::RightSide, 0);
}

ZimView* TabBar::createNewTab(bool setCurrent)
{
    auto tab = new ZimView(this, this);
    auto index = count() - 1;
    mp_stackedWidget->insertWidget(index, tab);
    insertTab(index, "");
    QToolButton *tb = new QToolButton(this);
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::CloseTabAction));
    setTabButton(index, QTabBar::RightSide, tb);
    if (setCurrent) {
        setCurrentIndex(index);
    }
    return tab;
}

void TabBar::openUrl(const QUrl& url, bool newTab)
{
    WebView* webView = currentWebView();
    if (newTab || !webView) {
        webView = createNewTab(true)->getWebView();
    }
    QUITIFNULL(webView);
    webView->setUrl(url);
}

void TabBar::setTitleOf(const QString& title, ZimView* tab)
{
    CURRENTIFNULL(tab);
    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        setTabText(mp_stackedWidget->indexOf(tab), url.path());
    } else {
        int idx = mp_stackedWidget->indexOf(tab);
        setTabToolTip(idx, title);

        // This logic is taken from the implementation:
        // <QTDIR>/5.12.6/Src/qtbase/src/widgets/widgets/qtabbar.cpp
        // void QTabBar::initStyleOption(QStyleOptionTab *option, int tabIndex) const
        QStyleOptionTab tab;
        initStyleOption(&tab, idx);
        QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, &tab, this);

        // but instead of eliding text as QTabBar::initStyleOption() does
        // we cut it and store the flag if it was cut
        QString cut = fontMetrics().elidedText(title, Qt::ElideRight, textRect.width());
        // strip ... from the end (this three dots are one char)
        if (cut.size() < title.size()) {
            cut = cut.mid(0, cut.size() - 1);
            // set flag that the text was too long, was cut and this tab
            // need 'fade out' effect while drawing
            setTabData(idx, QVariant::fromValue(true));
        }
        else {
            setTabData(idx, QVariant::fromValue(false));
        }
        setTabText(idx, cut);
    }
}

void TabBar::setIconOf(const QIcon &icon, ZimView *tab)
{
    CURRENTIFNULL(tab);
    setTabIcon(mp_stackedWidget->indexOf(tab), icon);
}

QString TabBar::currentZimId()
{
    if (!currentWidget())
        return "";
    return currentWebView()->zimId();
}

QString TabBar::currentArticleUrl()
{
    if(!currentWidget())
        return "";
    return currentWebView()->url().path();
}

QString TabBar::currentArticleTitle()
{
    if(!currentWidget())
        return "";
    return currentWebView()->title();
}

QSize TabBar::tabSizeHint(int index) const {
    if (index)
        return QSize(205, 40);
    return QSize(40, 40);
}

void TabBar::openFindInPageBar()
{
    currentWidget()->openFindInPageBar();    
}

void TabBar::triggerWebPageAction(QWebEnginePage::WebAction action, ZimView *widget)
{
    CURRENTIFNULL(widget);
    QUITIFNULL(widget);
    widget->getWebView()->triggerPageAction(action);
    widget->getWebView()->setFocus();
}

void TabBar::closeTab(int index)
{
    setSelectionBehaviorOnRemove(index);
    if (index == 0 || index == this->count() - 1)
        return;
    if (index == m_settingsIndex) {
        m_settingsIndex = -1;
    }
    if (index < m_settingsIndex) {
        m_settingsIndex--;
    }
    auto webview = widget(index);
    mp_stackedWidget->removeWidget(webview);
    webview->setParent(nullptr);
    removeTab(index);
    webview->close();
    delete webview;
}

void TabBar::setSelectionBehaviorOnRemove(int index)
{
    if (index == count() - 2) {
        setCurrentIndex(index - 1);
    } else {
        setCurrentIndex(index + 1);
    }
}

void TabBar::onCurrentChanged(int index)
{
    if (index == -1)
        return;
    if (index == m_settingsIndex) {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit libraryPageDisplayed(false);
        KiwixApp::instance()->setSideBar(KiwixApp::NONE);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    } else if (index) {
        auto view = widget(index)->getWebView();
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        emit libraryPageDisplayed(false);
        if (KiwixApp::instance()->getSideType() == KiwixApp::CONTENTMANAGER_BAR) {
            KiwixApp::instance()->setSideBar(KiwixApp::NONE);
        }
        QTimer::singleShot(0, [=](){emit currentTitleChanged(view->title());});
    } else {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit libraryPageDisplayed(true);
        KiwixApp::instance()->setSideBar(KiwixApp::CONTENTMANAGER_BAR);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    }
}

void TabBar::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        if (m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset(new FullScreenWindow(this->currentWebView()));
    } else {
        if (!m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset();
    }
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        closeTab(this->tabAt(event->pos()));
    } else {
       QTabBar::mousePressEvent(event);
    }
}

void TabBar::paintEvent(QPaintEvent *e)
{
    // Please keep it in sync with resources/css/style.css
    // QTabBar::tab:selected { background-color: <value> }
    const QColor selected_tab_bg_color = QColor(Qt::white);

    // first, let Qt draw QTabBar normally
    QTabBar::paintEvent(e);

    // Then apply fade-out effect for long tab title on top:
    QPainter p(this);

    for (int i = 0; i < count(); ++i) {
        bool need_fade_out = tabData(i).toBool();
        if (! need_fade_out)
            continue;

        QStyleOptionTab tab;
        initStyleOption(&tab, i);

        QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, &tab, this);

        QRect tail = textRect;
        tail.setWidth(textRect.width() * 0.2);

        // isRightToLeft() gives inherrited from application layout direction,
        // but we need the direction of each individual tab header text here
        bool right_to_left = tabText(i).isRightToLeft();

        if (! right_to_left) {
            // Normal left-to-right text layout: move fading-out box to the right
            tail.moveRight(textRect.right());
        }

        bool selected = tab.state & QStyle::State_Selected;

        /* This gets the color from our style.css rule:
         * QWidget {
         *   background-color: #EAECF0;
         * }
         */
        QColor c0 = tab.palette.background().color();

        if (selected) {
            /* We cannot just get back from QStyleSheetStyle (Qt private classes)
             * the value of QTabBar::tab:selected { background-color: <value> }
             * so have to use hard-coded value here:
             */
            c0 = selected_tab_bg_color;
        }

        QColor c1(c0);

        if (right_to_left) {
            c0.setAlpha(255);
            c1.setAlpha(0);
        }
        else {
            c0.setAlpha(0);
            c1.setAlpha(255);
        }

        QLinearGradient gr(tail.topLeft(), tail.topRight());
        gr.setSpread(QGradient::PadSpread);
        gr.setColorAt(0.0, c0);
        gr.setColorAt(1.0, c1);

        QBrush br(gr);
        p.fillRect(tail, br);
    }
}
