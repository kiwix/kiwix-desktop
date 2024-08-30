class QMenu;

#include "webview.h"

#include <QDesktopServices>
#include <QAction>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"
#include <QToolTip>
#include <QWebEngineSettings>
#include <QWebEngineHistory>
#include <QVBoxLayout>
#include <QFileDialog>
#include <zim/error.h>
#include <zim/item.h>

zim::Entry getArchiveEntryFromUrl(const zim::Archive& archive, const QUrl& url);

void WebViewBackMenu::showEvent(QShowEvent *)
{
    /* In Qt 5.12 CSS options for shifting this menu didn't work.
     * In particular:
     *   - toolbar->setContentsMargins(0,0,0,0);
     *   - toolbar->layout()->setContentsMargins(0,0,0,0);
     *   - QToolBar {   padding-left: }
     *   - QToolBar {   margin-left; }
     *   - QToolBar {   padding: 5px 12px 5px 12px; }
     *   - QToolBar::separator:first { width: 10px; }
     *  (that was attempts to set some spacing on left and right in toolbar
     *  so back button will be shifted right.
     *  If in Qt 6.x QToolButton shows its menu in the right position
     *  this code can be removed.
     */

    QRect geo = geometry();
    geo.moveLeft(geo.left() + 6);  // see also: style.css: QToolButton#backButton { margin-left: 6px; }
    geo.moveTop(geo.top() + 2);
    setGeometry(geo);
}

void WebViewForwardMenu::showEvent(QShowEvent *)
{
    QRect geo = geometry();
    geo.moveTop(geo.top() + 2);
    setGeometry(geo);
}

QString getZimIdFromUrl(QUrl url)
{
    return url.host().split('.')[0];
}

QString getResultTypeFromUrl(QUrl url)
{
    return url.host().split('.')[1];
}

void WebView::applyCorrectZoomFactor() {
    auto url = this->url();
    auto settingsManager = KiwixApp::instance()->getSettingsManager();
    qreal zoomFactor;
    const bool isSearchResultsView = QUrlQuery(url).hasQueryItem("pattern") && (getResultTypeFromUrl(url) == "search");
    if (isSearchResultsView) {
        zoomFactor = settingsManager->getZoomFactor();
    } else {
        auto zimId = getZimIdFromUrl(url);
        zoomFactor = settingsManager->getZoomFactorByZimId(zimId);
    }
    this->setZoomFactor(zoomFactor);
}

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebPage(this));
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::onUrlChanged);
    connect(this->page(), &QWebEnginePage::linkHovered, this, [=] (const QString& url) {
        m_linkHovered = url;
    });

    /* In Qt 5.12, the zoom factor is not correctly passed after a fulltext search
     * Bug Report: https://bugreports.qt.io/browse/QTBUG-51851
     * This rezooms the page to its correct zoom (default/by ZIM ID) after loading is finished.
     * If the page is search results, we put the default zoom factor
     * If in Qt 6.x, the bug is fixed this code can be removed.
     */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(this, &QWebEngineView::loadFinished, this, [=] (bool ok) {
        if (ok) {
            applyCorrectZoomFactor();
        }
    });
#endif
}

WebView::~WebView()
{}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QMenu* WebView::getHistoryBackMenu() const
{
    QWebEngineHistory *h = history();

    const int cur = h->currentItemIndex();
    if (cur <= 0) {
        return Q_NULLPTR;
    }

    auto ret = new WebViewBackMenu();
    for (int i = cur - 1 ; i >= 0 ; i--) {
        addHistoryItemAction(ret, h->itemAt(i), i);
    }
    return ret;
}

QMenu* WebView::getHistoryForwardMenu() const
{
    QWebEngineHistory *h = history();

    const int cur = h->currentItemIndex();
    if (cur + 1 >= h->count()) {
        return Q_NULLPTR;
    }

    auto ret = new WebViewForwardMenu();
    for (int i = cur + 1 ; i < h->count() ; i++) {
        addHistoryItemAction(ret, h->itemAt(i), i);
    }
    return ret;
}

void WebView::saveViewContent()
{
    try {
        auto app = KiwixApp::instance();
        auto library = app->getLibrary();
        auto archive = library->getArchive(m_currentZimId);
        auto entry = getArchiveEntryFromUrl(*archive, this->url());
        if (entry.isRedirect()) 
            return;

        auto item = entry.getItem(true);
        auto mimeType = QByteArray::fromStdString(item.getMimetype());
        mimeType = mimeType.split(';')[0];

        QString suggestedFileName = item.getTitle().c_str();
        if (mimeType == "text/html")
            page()->save(suggestedFileName + ".pdf");
        else
            page()->download(this->url(), suggestedFileName);
    }
    catch (...) { /* Blank */}
}

void WebView::addHistoryItemAction(QMenu *menu, const QWebEngineHistoryItem &item, int n) const
{
    QAction *a = menu->addAction(item.title());
    a->setData(QVariant::fromValue(n));
    connect(a, &QAction::triggered, this, &WebView::gotoTriggeredHistoryItemAction);
}

void WebView::gotoTriggeredHistoryItemAction()
{
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if (!a)
        return;

    int n = a->data().toInt();
    QWebEngineHistory *h = history();
    if (n < 0 || n >= h->count())
        return;

    h->goToItem(h->itemAt(n));
}


QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        return tabWidget->createNewTab(false, true)->getWebView();
    }
    return nullptr;
}

void WebView::onUrlChanged(const QUrl& url) {
    auto zimId = getZimIdFromUrl(url);
    auto app = KiwixApp::instance();
    app->saveListOfOpenTabs();
    if (m_currentZimId == zimId ) {
        return;
    }
    m_currentZimId = zimId;
    emit zimIdChanged(m_currentZimId);
    std::shared_ptr<zim::Archive> archive;
    try {
        archive = app->getLibrary()->getArchive(m_currentZimId);
    } catch (std::out_of_range& e) {
        return;
    }
    auto zoomFactor = app->getSettingsManager()->getZoomFactorByZimId(zimId);
    this->setZoomFactor(zoomFactor);
    try {
        std::string favicon, _mimetype;
        auto item = archive->getIllustrationItem(48);
        favicon = item.getData();
        _mimetype = item.getMimetype();
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)favicon.data(), favicon.size());
        m_icon = QIcon(pixmap);
        emit iconChanged(m_icon);
    } catch (zim::EntryNotFound& e) {}
}

void WebView::wheelEvent(QWheelEvent *event) {
    if ((event->modifiers() & Qt::ControlModifier) != 0)
    {
        if (event->angleDelta().y() > 0) {
            KiwixApp::instance()->getAction(KiwixApp::ZoomInAction)->activate(QAction::Trigger);
        } else if (event->angleDelta().y() < 0) {
            KiwixApp::instance()->getAction(KiwixApp::ZoomOutAction)->activate(QAction::Trigger);
        }
    }
}


void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu* menu;
    if (m_linkHovered.isEmpty()) {
        menu = createStandardContextMenu();
    } else {
        menu = createLinkContextMenu();
    }
    menu->exec(event->globalPos());
}


QMenu* WebView::createStandardContextMenu() {
    auto app = KiwixApp::instance();

    QMenu* menu = new QMenu(this);
    auto backAction = new QAction(gt("back"));
    backAction->setEnabled(app->getAction(KiwixApp::HistoryBackAction)->isEnabled());
    backAction->setIcon(app->getAction(KiwixApp::HistoryBackAction)->icon());
    menu->addAction(backAction);
    connect(menu, &QObject::destroyed, backAction, &QObject::deleteLater);
    connect(backAction, &QAction::triggered, this, [=](bool checked) {
        Q_UNUSED(checked);
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });

    auto forwardAction = new QAction(gt("forward"));
    forwardAction->setEnabled(app->getAction(KiwixApp::HistoryForwardAction)->isEnabled());
    forwardAction->setIcon(app->getAction(KiwixApp::HistoryForwardAction)->icon());
    menu->addAction(forwardAction);
    connect(menu, &QObject::destroyed, forwardAction, &QObject::deleteLater);
    connect(forwardAction, &QAction::triggered, this, [=](bool checked) {
        Q_UNUSED(checked);
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });

    return menu;
}


QMenu* WebView::createLinkContextMenu() {
    QMenu* menu = new QMenu(this);

    if (!m_linkHovered.startsWith("zim://")) {
        auto openLinkInWebBrowserAction = new QAction(gt("open-link-in-web-browser"));
        menu->addAction(openLinkInWebBrowserAction);
        connect(menu, &QObject::destroyed, openLinkInWebBrowserAction, &QObject::deleteLater);
        connect(openLinkInWebBrowserAction, &QAction::triggered, this, [=](bool checked) {
            Q_UNUSED(checked);
            QDesktopServices::openUrl(m_linkHovered);
        });
    } else {
        auto openLinkNewTab = new QAction(gt("open-link-new-tab"));
        openLinkNewTab->setIcon(QIcon(":/icons/new-tab-icon.svg"));
        menu->addAction(openLinkNewTab);
        connect(menu, &QObject::destroyed, openLinkNewTab, &QObject::deleteLater);
        connect(openLinkNewTab, &QAction::triggered, this, [=](bool checked) {
            Q_UNUSED(checked);
            KiwixApp::instance()->openUrl(m_linkHovered, true);
        });
    }

    return menu;
}


bool WebView::eventFilter(QObject *src, QEvent *e)
{
    Q_UNUSED(src)
    // work around QTBUG-43602
    if (e->type() == QEvent::Wheel) {
        auto we = static_cast<QWheelEvent *>(e);
        if (we->modifiers() == Qt::ControlModifier)
            return true;
    }
    if (e->type() == QEvent::MouseButtonRelease) {
        auto me = static_cast<QMouseEvent *>(e);
        if (!m_linkHovered.startsWith("zim://")
         && (me->modifiers() == Qt::ControlModifier || me->button() == Qt::MiddleButton))
        {
            QDesktopServices::openUrl(m_linkHovered);
            return true;
        }
        if (me->button() == Qt::BackButton)
        {
            back();
            return true;
        }
        if (me->button() == Qt::ForwardButton)
        {
            forward();
            return true;
        }
    }
    return false;
}

bool WebView::event(QEvent *event)
{
    // work around QTBUG-43602
    if (event->type() == QEvent::ChildAdded) {
        auto ce = static_cast<QChildEvent *>(event);
        ce->child()->installEventFilter(this);
    } else if (event->type() == QEvent::ChildRemoved) {
        auto ce = static_cast<QChildEvent *>(event);
        ce->child()->removeEventFilter(this);
    }
    if (event->type() == QEvent::ToolTip) {
        return true;
    } else {
        return QWebEngineView::event(event);
    }
    return true;
}
