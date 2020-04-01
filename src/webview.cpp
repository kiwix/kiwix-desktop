#include "webview.h"

#include <QDesktopServices>
#include <QAction>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"
#include <QToolTip>
#include <QWebEngineSettings>
#include <QVBoxLayout>

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

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        return tabWidget->createNewTab(type==QWebEnginePage::WebBrowserTab)->getWebView();
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
