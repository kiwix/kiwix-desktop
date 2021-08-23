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


WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebPage(this));
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::onUrlChanged);
    connect(this->page(), &QWebEnginePage::linkHovered, this, [=] (const QString& url) {
        m_linkHovered = url;
    });
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
    auto zimId = url.host().split('.')[0];
    if (m_currentZimId == zimId ) {
        return;
    }
    m_currentZimId = zimId;
    emit zimIdChanged(m_currentZimId);
    auto app = KiwixApp::instance();
    auto reader = app->getLibrary()->getReader(m_currentZimId);
    if (!reader) {
        return;
    }
    std::string favicon, _mimetype;
    reader->getFavicon(favicon, _mimetype);
    QPixmap pixmap;
    pixmap.loadFromData((const uchar*)favicon.data(), favicon.size());
    m_icon = QIcon(pixmap);
    emit iconChanged(m_icon);
    auto zoomFactor = app->getSettingsManager()->getZoomFactorByZimId(zimId);
    this->setZoomFactor(zoomFactor);
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
    auto menu = this->page()->createStandardContextMenu();
    pageAction(QWebEnginePage::OpenLinkInNewWindow)->setVisible(false);
    if (!m_linkHovered.isEmpty()) {
        if (!m_linkHovered.startsWith("zim://")) {
            pageAction(QWebEnginePage::OpenLinkInNewTab)->setVisible(false);
            auto openLinkInWebBrowserAction = new QAction(gt("open-link-in-web-browser"));
            menu->insertAction(pageAction(QWebEnginePage::DownloadLinkToDisk) , openLinkInWebBrowserAction);
            connect(menu, &QObject::destroyed, openLinkInWebBrowserAction, &QObject::deleteLater);
            connect(openLinkInWebBrowserAction, &QAction::triggered, this, [=](bool checked) {
                Q_UNUSED(checked);
                QDesktopServices::openUrl(m_linkHovered);
            });
        } else {
            pageAction(QWebEnginePage::OpenLinkInNewTab)->setVisible(true);
        }
    }
    menu->exec(event->globalPos());
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
