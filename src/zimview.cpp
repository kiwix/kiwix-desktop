#include "zimview.h"
#include "kiwixapp.h"
#include <QAction>
#include <QVBoxLayout>
#include <QToolTip>

ZimView::ZimView(TabBar *tabBar, QWidget *parent)
    : QWidget(parent),
      mp_tabBar(tabBar),
      mp_findInPageBar(new FindInPageBar(this))
{
    mp_webView = new WebView();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mp_webView);
    layout->addWidget(mp_findInPageBar);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout);
    mp_findInPageBar->hide();
    auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::ZoomInAction), &QAction::triggered,
            this, [=]() {
                auto zoomFactor = mp_webView->zoomFactor();
                zoomFactor += 0.1;
                zoomFactor = max(min(zoomFactor, 5.0), 0.25);
                mp_webView->setZoomFactor(zoomFactor);
                auto key = mp_webView->zimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomOutAction), &QAction::triggered,
            this, [=]() {
                auto zoomFactor = mp_webView->zoomFactor();
                zoomFactor -= 0.1;
                zoomFactor = max(min(zoomFactor, 5.0), 0.25);
                mp_webView->setZoomFactor(zoomFactor);
                auto key = mp_webView->zimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomResetAction), &QAction::triggered,
            this, [=]() {
                auto settingsManager = KiwixApp::instance()->getSettingsManager();
                mp_webView->setZoomFactor(settingsManager->getZoomFactor());
                auto key = mp_webView->zimId() + "/zoomFactor";
                settingsManager->deleteSettings(key);
            });
    connect(mp_webView->page(), &QWebEnginePage::fullScreenRequested, mp_tabBar, &TabBar::fullScreenRequested);
    connect(mp_webView, &WebView::titleChanged, this,
            [=](const QString& str) {
                mp_tabBar->setTitleOf(str, this);
                if (mp_tabBar->currentZimView() != this) {
                    return;
                }
                emit mp_tabBar->currentTitleChanged(str);
            });
    connect(mp_webView, &WebView::iconChanged, this,
            [=](const QIcon& icon) { mp_tabBar->setIconOf(icon, this); });
    connect(mp_webView, &WebView::zimIdChanged, this,
            [=](const QString& zimId) {
                if (mp_tabBar->currentZimView() != this) {
                    return;
                }
                emit mp_tabBar->currentZimIdChanged(zimId);
            });
    connect(mp_webView->page()->action(QWebEnginePage::Back), &QAction::changed,
            [=]() {
                if (mp_tabBar->currentZimView() != this) {
                    return;
                }
                emit mp_tabBar->webActionEnabledChanged(QWebEnginePage::Back, mp_webView->isWebActionEnabled(QWebEnginePage::Back));
            });
    connect(mp_webView->page()->action(QWebEnginePage::Forward), &QAction::changed,
            [=]() {
                if (mp_tabBar->currentZimView() != this) {
                    return;
                }
                emit mp_tabBar->webActionEnabledChanged(QWebEnginePage::Forward, mp_webView->isWebActionEnabled(QWebEnginePage::Forward));
            });
    connect(mp_webView->page(), &QWebEnginePage::linkHovered, this,
            [=](const QString& url) {
                if (mp_tabBar->currentIndex() == 0) {
                    return;
                }
                if (url.isEmpty()) {
                    QToolTip::hideText();
                } else {
                    auto link = url;
                    if (url.startsWith("zim://")) {
                        link = QUrl(url).path();
                    }

                    /* because we use QWebEnginePage::linkHovered signal,
                     * we can be sure the current tab is a web page
                     * (mp_tabBar->currentWebView() is not nullptr)
                     */
                    auto height = mp_tabBar->currentWebView()->height() + 1;

                    auto pos = mp_tabBar->mapToGlobal(QPoint(-3, height));
                    QToolTip::showText(pos, link);
                }
            });
}

void ZimView::openFindInPageBar()
{
    mp_findInPageBar->show();
    mp_findInPageBar->getFindLineEdit()->setFocus();
}
