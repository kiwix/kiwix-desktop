class QMenu;

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
#define CURRENTIFNULL(VIEW) if(nullptr==VIEW) { VIEW = currentZimView();}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent)
{
    QTabBar::setDrawBase(false);
    setTabsClosable(true);
    setElideMode(Qt::ElideNone);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    setMovable(true);
    setIconSize(QSize(30, 30));
    connect(this, &QTabBar::currentChanged, this, &TabBar::onCurrentChanged, Qt::QueuedConnection);
    auto app = KiwixApp::instance();

    connect(app->getAction(KiwixApp::NextTabAction), &QAction::triggered, this, &TabBar::moveToNextTab);
    connect(app->getAction(KiwixApp::PreviousTabAction), &QAction::triggered, this, &TabBar::moveToPreviousTab);
    connect(app->getAction(KiwixApp::NewTabAction), &QAction::triggered,
            this, [=]() {
                this->createNewTab(true, false);
                auto topWidget = KiwixApp::instance()->getMainWindow()->getTopWidget();
                topWidget->getSearchBar().setFocus(Qt::MouseFocusReason);
                topWidget->getSearchBar().clear();
                topWidget->getSearchBar().clearSuggestions();
                topWidget->getSearchBar().hideSuggestions();
          });
    connect(app->getAction(KiwixApp::CloseTabAction), &QAction::triggered,
            this, [=]() {
                auto index = currentIndex();
                if (index < 0)
                    return;

                // library tab cannot be closed
                QWidget *w = mp_stackedWidget->widget(index);
                if (qobject_cast<ContentManagerView*>(w)) {
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
                SettingsView* view = KiwixApp::instance()->getSettingsManager()->getView();
                for (int i = 0 ; i < mp_stackedWidget->count(); i++) {
                    if (mp_stackedWidget->widget(i) == view) {
                        setCurrentIndex(i);
                        return;
                    }
                }
                int index = currentIndex() + 1;
                mp_stackedWidget->insertWidget(index, view);
                emit tabDisplayed(TabType::SettingsTab);
                insertTab(index,QIcon(":/icons/settings.svg"), gt("settings"));
                QToolButton *tb = new QToolButton(this);
                tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::CloseTabAction));
                setTabButton(index, QTabBar::RightSide, tb);
                setCurrentIndex(index);
            });

    for (int i = 0 ; i <= 9 ; i++) {
        QAction *a = new QAction(this);
        a->setData(QVariant::fromValue(i));
        QKeySequence ks(Qt::ALT | (Qt::Key_0 + i));
        a->setShortcut(ks);
        addAction(a);
        connect(a, &QAction::triggered, this, [=](){
            QAction *a = qobject_cast<QAction*>(QObject::sender());
            if (!a)
                return;

            bool ok;
            int tab_n = a->data().toInt(&ok);
            if (tab_n==0)
                tab_n=10;
            if (!ok)
                return;
            if (tab_n >= count())
                return;

            setCurrentIndex(tab_n-1);
        });
    }

    // the slot relies the connection will be direct to reverting back the tab
    connect(this, SIGNAL(tabMoved(int,int)),
            this, SLOT(onTabMoved(int,int)), Qt::DirectConnection);
}

void TabBar::setStackedWidget(QStackedWidget *widget) {
    mp_stackedWidget = widget;
    connect(this, &QTabBar::currentChanged,
            widget, &QStackedWidget::setCurrentIndex);
}

void TabBar::setContentManagerView(ContentManagerView* view)
{
    qInfo() << "add widget";
    mp_stackedWidget->addWidget(view);
    mp_stackedWidget->show();
    int idx = addTab(QIcon(":/icons/kiwix-logo.svg"), "");
    setTabButton(idx, RightSide, nullptr);
}

void TabBar::setNewTabButton()
{
    QToolButton *tb = new QToolButton();
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::NewTabAction));
    tb->setIcon(QIcon(":/icons/new-tab-icon.svg"));
    int idx = addTab("");
    setTabEnabled(idx, false);
    setTabButton(idx, QTabBar::LeftSide, tb);
    tabButton(idx, QTabBar::RightSide)->deleteLater();
    setTabButton(idx, QTabBar::RightSide, Q_NULLPTR);
}

int TabBar::realTabCount() const
{
    // The last tab is "+" in TabBar, but that isn't a real tab which displays any content hence the real count is tab count - 1
    if (count() < 1)
        return 0;
    return count() - 1;
}

void TabBar::moveToNextTab()
{
    const int index = currentIndex();
    setCurrentIndex(index == realTabCount() - 1 ? 0 : index + 1);
}

void TabBar::moveToPreviousTab()
{
    const int index = currentIndex();
    setCurrentIndex(index <= 0 ? realTabCount() - 1 : index - 1);
}

ZimView* TabBar::createNewTab(bool setCurrent, bool adjacentToCurrentTab)
{
    auto tab = new ZimView(this, this);
    int index;
    if(adjacentToCurrentTab) {
        index = currentIndex() + 1;
    } else {
        index = realTabCount(); // for New Tab Button
    }
    mp_stackedWidget->insertWidget(index, tab);
    index = insertTab(index, "");
    QToolButton *tb = new QToolButton(this);
    tb->setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::CloseTabAction));
    setTabButton(index, QTabBar::RightSide, tb);
    if (setCurrent) {
        setCurrentIndex(index);
    }

    connect(tab, &ZimView::webActionEnabledChanged,
            this, &TabBar::onWebviewHistoryActionChanged);

    return tab;
}

void TabBar::openUrl(const QUrl& url, bool newTab)
{
    WebView* webView = currentWebView();
    if (newTab || !webView) {
        webView = createNewTab(true, true)->getWebView();
    }
    QUITIFNULL(webView);
    webView->setUrl(url);
}

void TabBar::setTitleOf(const QString& title, ZimView* tab)
{
    CURRENTIFNULL(tab);
    int idx = mp_stackedWidget->indexOf(tab);
    if (idx < 0)
        return;

    QString t = title;

    if (title.startsWith("zim://")) {
        auto url = QUrl(title);
        t = url.path();
    }

    setTabToolTip(idx, t);

    /* we don't use setTabText() because Qt can't draw text with
     * fade-out gradient selectively (only for large texts).
     * So we just store tab's title, and will draw it later ourselves.
     */
    setTabData(idx, QVariant::fromValue(t));

    // need to initiate repaint
    // because setTabData() didn't do it as setTabText() before.
    repaint();
}

void TabBar::setIconOf(const QIcon &icon, ZimView *tab)
{
    CURRENTIFNULL(tab);
    setTabIcon(mp_stackedWidget->indexOf(tab), icon);
}

QString TabBar::currentZimId()
{
    if (WebView *w = currentWebView())
        return w->zimId();
    return "";
}

QString TabBar::currentArticleUrl()
{
    if (WebView *w = currentWebView())
        return w->url().path();
    return "";
}

QString TabBar::currentArticleTitle()
{
    if (WebView *w = currentWebView())
        return w->title();
    return "";
}

QSize TabBar::tabSizeHint(int index) const
{
    QWidget *w = mp_stackedWidget->widget(index);

    if (w && qobject_cast<ContentManagerView*>(w))
        return QSize(40, 40); // the "Library" tab is only icon

    return QSize(205, 40); // "Settings" and content tabs have text
}

void TabBar::openFindInPageBar()
{
    if (ZimView *zv = currentZimView())
        zv->openFindInPageBar();
}

void TabBar::triggerWebPageAction(QWebEnginePage::WebAction action, ZimView *widget)
{
    CURRENTIFNULL(widget);
    QUITIFNULL(widget);
    widget->getWebView()->triggerPageAction(action);
    widget->getWebView()->setFocus();
}

void TabBar::closeTabsByZimId(const QString &id)
{
    // the last tab is + button, skip it
    for (int i = count() - 2 ; i >= 0 ; i--) {
        auto *zv = qobject_cast<ZimView*>(mp_stackedWidget->widget(i));
        if (!zv)
            continue;
        if (zv->getWebView()->zimId() == id) {
            closeTab(i);
        }
    }
}

void TabBar::closeTab(int index)
{
    // the last tab is + button, cannot be closed
    if (index == this->realTabCount())
        return;

    setSelectionBehaviorOnRemove(index);

    QWidget *view = mp_stackedWidget->widget(index);

    // library tab cannot be closed
    if (qobject_cast<ContentManagerView*>(view)) {
        return;
    }

    mp_stackedWidget->removeWidget(view);
    view->setParent(nullptr);
    removeTab(index);
    view->close();
    view->deleteLater();
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

    // if somehow the last tab (+ button) became active, switch to the previous
    if (index >= realTabCount()) {
        setCurrentIndex(realTabCount() - 1);
        return;
    }

    QWidget *w = mp_stackedWidget->widget(index);

    if (qobject_cast<SettingsView*>(w)) {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit tabDisplayed(TabType::SettingsTab);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    } else if (auto zv = qobject_cast<ZimView*>(w)) {
        auto view = zv->getWebView();
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        emit tabDisplayed(TabType::ZimViewTab);
        QTimer::singleShot(0, [=](){emit currentTitleChanged(view->title());});
    } else if (qobject_cast<ContentManagerView*>(w)) {
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit tabDisplayed(TabType::LibraryTab);
        QTimer::singleShot(0, [=](){emit currentTitleChanged("");});
    }
    else {
        Q_ASSERT(false);
        // In the future, other types of tabs can be added.
        // For example, About dialog, or Kiwix Server control panel.
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

void TabBar::on_webview_titleChanged(const QString& title)
{
    ZimView* tab = qobject_cast<ZimView*>(sender()->parent());
    if (! tab)
        return;

    setTitleOf(title, tab);

    if (currentZimView() == tab)
        emit currentTitleChanged(title);
}

void TabBar::onWebviewHistoryActionChanged(QWebEnginePage::WebAction action, bool enabled)
{
    ZimView *zv = qobject_cast<ZimView*>(sender());

    if (!zv || zv != this->currentZimView())
        return;

    emit webActionEnabledChanged(action, enabled);
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
    /* first, let Qt draw QTabBar normally.
     * QTabbar will leave tabs titles empty because we didn't set these values into QTabBar.
     */
    QTabBar::paintEvent(e);

    // Then, for each tab, we draw titles, using fade-out effect when needed.
    QPainter p(this);

    for (int i = 0; i < count(); ++i) {
        QString tab_title = tabData(i).toString();
        if (tab_title.isEmpty())
            continue;

        /* See the implementation of QTabBar for better understanding this code:
         * <QTDIR>/5.12.6/Src/qtbase/src/widgets/widgets/qtabbar.cpp
         * in particular, QTabBar::initStyleOption()
         */
        QStyleOptionTab tabopt;
        initStyleOption(&tabopt, i);

        bool need_fade_out = false;

        QRect tabTextRect = style()->subElementRect(QStyle::SE_TabBarTabText, &tabopt, this);

        QRect fontTextRect = fontMetrics().boundingRect(tab_title);

        if (fontTextRect.width() > tabTextRect.width())
            need_fade_out = true;

        bool right_to_left = tab_title.isRightToLeft();

        if (need_fade_out) {
            // draw the most of tab text extent with the normal color,
            // and draw the rest with alpha channel gradient
            QColor c0 = tabopt.palette.brush(QPalette::ButtonText).color();
            QColor c1(c0);

            c0.setAlpha(255);   // color of font
            c1.setAlpha(0);     // transparent

            const int mid_Y = tabTextRect.center().y();
            QLinearGradient gr;

            if (right_to_left) {
                // arabic right-to-left text
                gr.setStart(tabTextRect.x(), mid_Y);
                gr.setFinalStop(tabTextRect.x() + 0.2 * tabTextRect.width(), mid_Y);
                gr.setColorAt(0.0, c1);
                gr.setColorAt(1.0, c0);
            }
            else {
                // normal left-to-right text direction
                gr.setStart(tabTextRect.x() + 0.8 * tabTextRect.width(), mid_Y);
                gr.setFinalStop(tabTextRect.right(), mid_Y);
                gr.setColorAt(0.0, c0);
                gr.setColorAt(1.0, c1);
            }
            tabopt.palette.setBrush(QPalette::ButtonText, QBrush(gr));
        }

        int align = Qt::AlignVCenter;
        if (need_fade_out) {
            align |= (right_to_left)? Qt::AlignRight : Qt::AlignLeft;
        }

        style()->drawItemText(&p, tabTextRect, align,
                  tabopt.palette, true, tab_title, QPalette::ButtonText);
    }
}

void TabBar::onTabMoved(int from, int to)
{
    // avoid infinitive recursion
    static bool reverting = false;
    if (reverting)
        return;

    // on attempt to move the last tab (+ button) just move it back
    // and the library should stick the first (zero) tab
    int last = mp_stackedWidget->count();
    if (
         (from == last || to == last) ||
         (from == 0 || to == 0)
       ) {
        reverting = true;
        moveTab(to, from);
        reverting = false;
        return;
    }

    // swap widgets
    QWidget *w_from = mp_stackedWidget->widget(from);
    mp_stackedWidget->removeWidget(w_from);
    mp_stackedWidget->insertWidget(to, w_from);
}
