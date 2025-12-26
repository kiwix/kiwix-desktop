class QMenu;

#include "webview.h"

#include <QDesktopServices>
#include <QAction>
#include <QClipboard>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"
#include "css_constants.h"
#include <QToolTip>
#include <QWebEngineSettings>
#include <QWebEngineHistory>
#include <QVBoxLayout>
#include <QFileDialog>
#include <zim/error.h>
#include <zim/item.h>
#include <kiwix/tools.h>
#include <QWebChannel>
#include <QWebEngineScript>
#include "kiwixwebchannelobject.h"
#include "tableofcontentbar.h"
#include <QTimer>

zim::Entry getArchiveEntryFromUrl(const zim::Archive& archive, const QUrl& url);
QString askForSaveFilePath(const QString& suggestedName);

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

    const int marginLeft = CSS::TopWidget::QToolButton::backButton::marginLeft;
    geo.moveLeft(geo.left() + marginLeft);
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
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::handleTocHistoryNavigation);
    
    // Inject JavaScript to handle popstate events for better history navigation
    connect(page(), &QWebEnginePage::loadStarted, this, [=]() {
        // Inject a popstate event handler to ensure smooth navigation and scrolling
        QString js =
        "if (!window._kiwixHandlerInstalled) {"
        "  window._kiwixHandlerInstalled = true;"
        "  try {"
        "    window.addEventListener('popstate', function(event) {"
        "      if (location.hash) {"
        "        var anchorId = location.hash.substring(1);"
        "        var element = document.getElementById(anchorId);"
        "        if (element) {"
        "          element.scrollIntoView({behavior: 'smooth'});"
        "        }"
        "      }"
        "    });"
        "  } catch(e) {"
        "    console.error('Error in popstate handler:', e);"
        "  }"
        "}";
        
        page()->runJavaScript(js);
    });
    
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

    const auto channel = new QWebChannel(this);
    const auto kiwixChannelObj = new KiwixWebChannelObject;
    page()->setWebChannel(channel, QWebEngineScript::UserWorld);
    channel->registerObject("kiwixChannelObj", kiwixChannelObj);

    const auto tabbar = KiwixApp::instance()->getTabWidget();
    connect(tabbar, &TabBar::currentTitleChanged, this, &WebView::onCurrentTitleChanged);
    connect(kiwixChannelObj, &KiwixWebChannelObject::headersChanged, this, &WebView::onHeadersReceived);
    connect(kiwixChannelObj, &KiwixWebChannelObject::navigationRequested,
            this, &WebView::onNavigationRequested);
    connect(kiwixChannelObj, &KiwixWebChannelObject::consoleMessageReceived,
            this, &WebView::onConsoleMessageReceived);

    const auto tocbar = KiwixApp::instance()->getMainWindow()->getTableOfContentBar();
    connect(this, &WebView::headersChanged, tocbar, &TableOfContentBar::setupTree);
    connect(tocbar, &TableOfContentBar::navigationRequested, this, &WebView::onNavigationRequested);
    connect(this, &WebView::navigationRequested, kiwixChannelObj, &KiwixWebChannelObject::navigationRequested);

    // Add this in the WebView constructor
    connect(this, &QWebEngineView::loadFinished, this, [this](bool success) {
        if (success) {
            // Update history action states
            auto app = KiwixApp::instance();
            app->getAction(KiwixApp::HistoryBackAction)->setEnabled(history()->canGoBack());
            app->getAction(KiwixApp::HistoryForwardAction)->setEnabled(history()->canGoForward());
            
            // This will catch ALL successful page loads, including history navigation
            // Update TOC selection if URL has fragment
            if (url().hasFragment()) {
                QString fragment = url().fragment();
                qDebug() << "Page load finished - Updating TOC selection for fragment:" << fragment;
                
                // Use a slightly longer delay for more reliability
                QTimer::singleShot(250, this, [this, fragment]() {
                    auto app = KiwixApp::instance();
                    auto tocBar = app->getMainWindow()->getTableOfContentBar();
                    if (tocBar) {
                        qDebug() << "Applying TOC selection update for fragment:" << fragment;
                        tocBar->updateSelectionFromFragment(fragment);
                        
                        // Force another scroll to the anchor to ensure it's visible
                        page()->runJavaScript(QString(
                            "if (document.getElementById('%1')) {"
                            "   console.log('Page loaded - Ensuring scroll to anchor: %1');"
                            "   document.getElementById('%1').scrollIntoView({behavior: 'smooth'});"
                            "}"
                        ).arg(fragment));
                    }
                });
            }
        }
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

namespace
{

/**
 * @brief Get the Zim Item object corresponding to the given url.
 * 
 * @param url QUrl
 * @return zim::Item
 * 
 * @exception throws exception if zimId is invalid, archive doesn't exist,
 * entry is invalid or not found, or entry is redirect.
 */
zim::Item getZimItem(const QUrl& url)
{
    const auto app = KiwixApp::instance();
    const auto library = app->getLibrary();
    const auto archive = library->getArchive(getZimIdFromUrl(url));
    const auto entry = getArchiveEntryFromUrl(*archive, url);
    return entry.getItem(true);
}

bool isHTMLContent(const zim::Item& item)
{
    auto mimeType = QByteArray::fromStdString(item.getMimetype());
    mimeType = mimeType.split(';')[0];
    return mimeType == "text/html";
}

}

void WebView::saveViewContent()
{
    try {
        const auto item = getZimItem(url());

        /* We have to sanitize here, as parsing will start once we pass the file
           name to either save or download method.
        */
        const QString suggestedFileName = QString::fromStdString(kiwix::getSlugifiedFileName(item.getTitle()));
        if (isHTMLContent(item))
        {
            const QString fileName = askForSaveFilePath(suggestedFileName + ".pdf");
            if (!fileName.isEmpty())
                page()->printToPdf(fileName);
        }
        else
            page()->download(this->url(), suggestedFileName);
    }
    catch (...) { /* Blank */}
}

void WebView::onCurrentTitleChanged()
{
    const auto tabbar = KiwixApp::instance()->getTabWidget();
    const auto noAnchorUrl = url().url(QUrl::RemoveFragment);
    const auto headersValid = m_headers["url"].toString() == noAnchorUrl;

    /* If headers not valid for this webview, then we are loading and the emit 
       will be handled by KiwixWebChannelObject::headersChanged.
    */
    if (tabbar->currentWebView() == this && headersValid)
        emit headersChanged(m_headers);
}

void WebView::onHeadersReceived(const QString& headersJSONStr)
{
    QJsonDocument doc = QJsonDocument::fromJson(headersJSONStr.toUtf8());
    if (!doc.isObject())
        return;

    m_headers = doc.object();
    if (KiwixApp::instance()->getTabWidget()->currentWebView() == this)
        emit headersChanged(m_headers);
}

void WebView::onConsoleMessageReceived(const QString& message)
{
    // Parse the JSON message
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject())
        return;

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QString msg = obj["message"].toString();

    // Special handling for history navigation messages
    if (type == "history-navigation") {
        // Check if we have an anchor in the JSON
        if (obj.contains("anchor") && obj["anchor"].isString()) {
            QString anchor = obj["anchor"].toString();
            
            if (!anchor.isEmpty()) {
                // Sync TOC selection with the current fragment
                QTimer::singleShot(10, this, [this, anchor]() {
                    syncTOCWithFragment(anchor);
                });
            }
        }
    }
}

void WebView::onNavigationRequested(const QString &url, const QString &anchor)
{
    // Safety checks
    const auto tabbar = KiwixApp::instance()->getTabWidget();
    if (!tabbar || tabbar->currentWebView() != this) {
        return;
    }

    // Create a QUrl with fragment for history
    QUrl historyUrl(url);
    historyUrl.setFragment(anchor);

    // Update TOC selection
    auto app = KiwixApp::instance();
    auto tocBar = app->getMainWindow()->getTableOfContentBar();
    if (tocBar) {
        tocBar->updateSelectionFromFragment(anchor);
    }

    // Check if we're already at this anchor
    if (this->url().hasFragment() && this->url().fragment() == anchor) {
        return;
    }

    // Properly escape the URL and anchor for JavaScript
    QString escapedUrl = url.toHtmlEscaped();
    QString escapedAnchor = anchor.toHtmlEscaped();

    // JS code to avoid Unexpected end of input error
   QString js = QString(
        "try {"
        "  if (window.history && window.history.pushState) {"
        "    var elem = document.getElementById('%1');"
        "    if (elem) {"
        "      window.history.pushState({anchor: '%1'}, '', '%2#%1');"
        "      elem.scrollIntoView({behavior: 'smooth'});"
        "    } else {"
        "      console.error('Anchor not found: %1');"
        "    }"
        "  }"
        "} catch(e) {"
        "  console.error('Navigation error:', e);"
        "}"
    ).arg(escapedAnchor).arg(escapedUrl);
    // Execute JavaScript safely with a callback
    page()->runJavaScript(js, [this, url, anchor](const QVariant &result) {
        Q_UNUSED(result);
        // Emit the navigation signal to the JavaScript after the history is updated
        // Use a small delay to prevent navigation loops
        QTimer::singleShot(50, this, [this, url, anchor]() {
            emit navigationRequested(url, anchor);
        });
    });
}

// Add a method to handle history navigation for TOC entries
void WebView::handleTocHistoryNavigation(const QUrl &url)
{
    // Safety check
    if (!url.isValid()) {
        return;
    }

    // For any URL with a fragment, update TOC selection
    if (url.hasFragment()) {
        QString anchor = url.fragment();
        
        // Safety check for empty anchor
        if (anchor.isEmpty()) {
            return;
        }
        
        // Update TOC selection for any URL with a fragment
        auto app = KiwixApp::instance();
        auto tocBar = app->getMainWindow()->getTableOfContentBar();
        if (tocBar) {
            tocBar->updateSelectionFromFragment(anchor);
        }
        
        // Only for URLs on the current page, emit navigation signal
        if (url.url(QUrl::RemoveFragment) == this->url().url(QUrl::RemoveFragment)) {
            // Only emit navigation signal if we're not already at this anchor
            if (!(this->url().hasFragment() && this->url().fragment() == anchor)) {
                // Use a small delay to prevent navigation loops
                QTimer::singleShot(50, this, [this, url, anchor]() {
                    // Emit navigation signal instead of loading the page
                    emit navigationRequested(url.url(QUrl::RemoveFragment), anchor);
                });
            }
        }
    }
}

void WebView::addHistoryItemAction(QMenu *menu,
                                   const QWebEngineHistoryItem &item,
                                   int n) const
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

    // Store the item before navigating
    QWebEngineHistoryItem item = h->itemAt(n);
    
    // If the target URL has a fragment, update TOC after navigation
    if (item.url().hasFragment()) {
        QString fragment = item.url().fragment();
        
        // After navigation, update the TOC selection
        QTimer::singleShot(200, this, [this, fragment]() {
            auto app = KiwixApp::instance();
            auto tocBar = app->getMainWindow()->getTableOfContentBar();
            if (tocBar) {
                tocBar->updateSelectionFromFragment(fragment);
            }
        });
    }
    
    // Go to the history item
    h->goToItem(item);
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
        // Even if the ZIM ID hasn't changed, we still need to update TOC selection
        if (url.hasFragment()) {
            QString fragment = url.fragment();
            
            // Sync TOC selection after a short delay to ensure the page has loaded
            QTimer::singleShot(100, this, [this, fragment]() {
                syncTOCWithFragment(fragment);
            });
        }
        return;
    }
    m_currentZimId = zimId;
    emit zimIdChanged(m_currentZimId);
    m_icon = app->getLibrary()->getBookIcon(m_currentZimId);
    auto zoomFactor = app->getSettingsManager()->getZoomFactorByZimId(zimId);
    this->setZoomFactor(zoomFactor);
    emit iconChanged(m_icon);

    // Update history action states
    app->getAction(KiwixApp::HistoryBackAction)->setEnabled(history()->canGoBack());
    app->getAction(KiwixApp::HistoryForwardAction)->setEnabled(history()->canGoForward());
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

#if defined(QT_TEXTTOSPEECH_LIB)
    const auto app = KiwixApp::instance();
    menu->addAction(app->getAction(KiwixApp::ReadArticleAction));
    if (page()->hasSelection())
        menu->addAction(app->getAction(KiwixApp::ReadTextAction));
#endif

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

    menu->addAction(app->getAction(KiwixApp::SavePageAsAction));
    return menu;
}


QMenu* WebView::createLinkContextMenu() {
    QMenu* menu = new QMenu(this);

    if (!m_linkHovered.startsWith("zim://")) {
        auto copyLinkAction = new QAction(gt("copy-link"));
        menu->addAction(copyLinkAction);
        connect(menu, &QObject::destroyed, copyLinkAction, &QObject::deleteLater);
        connect(copyLinkAction, &QAction::triggered, this, [=](bool checked) {
            Q_UNUSED(checked);
            QApplication::clipboard()->setText(m_linkHovered);
        });
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

void WebView::syncTOCWithFragment(const QString& fragment)
{
    if (fragment.isEmpty()) {
        return;
    }
    
    // Get the TOC bar and update selection
    auto app = KiwixApp::instance();
    auto tocBar = app->getMainWindow()->getTableOfContentBar();
    if (tocBar) {
        // Update the TOC selection to match the current fragment
        tocBar->updateSelectionFromFragment(fragment);
        
        // Ensure the fragment is properly scrolled into view
        QString escapedFragment = fragment.toHtmlEscaped();
        QString js = QString(
            "try {"
            "  var element = document.getElementById('%1');"
            "  if (element) {"
            "    element.scrollIntoView({behavior: 'smooth'});"
            "    setTimeout(function() {"
            "      element.scrollIntoView({behavior: 'smooth'});"
            "    }, 100);"
            "  }"
            "} catch(e) { console.error('Error in syncTOCWithFragment:', e); }"
        ).arg(escapedFragment);
        
        // Execute the JavaScript
        page()->runJavaScript(js);
    }
}