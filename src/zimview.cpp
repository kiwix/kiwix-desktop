#include "zimview.h"
#include "kiwixapp.h"
#include <QAction>
#include <QVBoxLayout>
#include <QToolTip>
#include <kiwix/tools.h>
#if defined(QT_TEXTTOSPEECH_LIB)
#include "texttospeechbar.h"
#endif

ZimView::ZimView(TabBar *tabBar, QWidget *parent)
    : QWidget(parent),
      mp_tabBar(tabBar),
      mp_findInPageBar(new FindInPageBar(this))
{
    auto app = KiwixApp::instance();
    mp_webView = new WebView();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mp_webView);
#if defined(QT_TEXTTOSPEECH_LIB)
    mp_ttsBar = new TextToSpeechBar(this);
    layout->addWidget(mp_ttsBar);
    connect(app->getAction(KiwixApp::ReadArticleAction), &QAction::triggered,
            this, [=]() {
                if (mp_tabBar->currentZimView() != this)
                    return;
                mp_webView->page()->toPlainText([=](const QString& articleText){
                    mp_ttsBar->speak(articleText);
                });
            });
    connect(app->getAction(KiwixApp::ReadTextAction), &QAction::triggered,
            this, [=]() {
                if (mp_tabBar->currentZimView() != this || !mp_webView->page()->hasSelection())
                    return;
                mp_ttsBar->speak(mp_webView->page()->selectedText());
            });
    connect(app->getAction(KiwixApp::ReadTextAction), &QAction::triggered, this, [=](){
        if (mp_tabBar->currentZimView() != this)
            return;
        if (this->getWebView()->page()->hasSelection())
            mp_ttsBar->show();
    });
    connect(app->getAction(KiwixApp::ReadArticleAction), &QAction::triggered, this, [=](){
        if (mp_tabBar->currentZimView() == this)
            mp_ttsBar->show();
    });
    connect(mp_webView, &WebView::zimIdChanged, mp_ttsBar, [=]{
        try {
            auto book = app->getLibrary()->getBookById(mp_tabBar->currentZimId());
            auto iso2 = QString::fromStdString(book.getLanguages().at(0)).chopped(1);
            auto iso3 = QString::fromStdString(book.getLanguages().at(0));
            
            /* Try both 3 letter and two letter name. */
            auto cLocale = QLocale(QLocale::C);
            auto locale = QLocale(iso2).language() == cLocale.language() ? QLocale(iso3) : QLocale(iso2);
            mp_ttsBar->setLocale(locale);
        } catch (...) { /* Blank */ }
    });
#endif
    
    layout->addWidget(mp_findInPageBar);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout); // now 'mp_webView' has 'this' as the parent QObject
    mp_findInPageBar->hide();
    connect(app->getAction(KiwixApp::ZoomInAction), &QAction::triggered,
            this, [=]() {
                if (mp_tabBar->currentZimView() != this)
                    return;
                auto zoomFactor = mp_webView->zoomFactor();
                zoomFactor += 0.1;
                zoomFactor = std::max(std::min(zoomFactor, 5.0), 0.25);
                mp_webView->setZoomFactor(zoomFactor);
                auto key = mp_webView->zimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomOutAction), &QAction::triggered,
            this, [=]() {
                if (mp_tabBar->currentZimView() != this)
                    return;
                auto zoomFactor = mp_webView->zoomFactor();
                zoomFactor -= 0.1;
                zoomFactor = std::max(std::min(zoomFactor, 5.0), 0.25);
                mp_webView->setZoomFactor(zoomFactor);
                auto key = mp_webView->zimId() + "/zoomFactor";
                KiwixApp::instance()->getSettingsManager()->setSettings(key, zoomFactor);
            });
    connect(app->getAction(KiwixApp::ZoomResetAction), &QAction::triggered,
            this, [=]() {
                if (mp_tabBar->currentZimView() != this)
                    return;
                auto settingsManager = KiwixApp::instance()->getSettingsManager();
                mp_webView->setZoomFactor(settingsManager->getZoomFactor());
                auto key = mp_webView->zimId() + "/zoomFactor";
                settingsManager->deleteSettings(key);
            });
    connect(app->getAction(KiwixApp::SavePageAsAction), &QAction::triggered, 
            this, [=](){
                if (mp_tabBar->currentZimView() == this)
                    mp_webView->saveViewContent();
            });
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::zoomChanged, this, [=]() {
        auto key = mp_webView->zimId() + "/zoomFactor";
        auto zimZoomFactor = KiwixApp::instance()->getSettingsManager()->getSettings(key);
        if(!zimZoomFactor.toBool()) {
            mp_webView->setZoomFactor(KiwixApp::instance()->getSettingsManager()->getZoomFactor());
        }
    });
    connect(mp_webView->page(), &QWebEnginePage::fullScreenRequested, mp_tabBar, &TabBar::fullScreenRequested);
    connect(mp_webView, &WebView::titleChanged, mp_tabBar, &TabBar::on_webview_titleChanged);
    connect(mp_webView, &WebView::iconChanged, this,
            [=](const QIcon& icon) { mp_tabBar->setIconOf(icon, this); });

    connect(mp_webView->page()->action(QWebEnginePage::Back), &QAction::changed,
            [this]() {
                emit webActionEnabledChanged(QWebEnginePage::Back, this->mp_webView->isWebActionEnabled(QWebEnginePage::Back));
            });
    connect(mp_webView->page()->action(QWebEnginePage::Forward), &QAction::changed,
            [this]() {
                emit webActionEnabledChanged(QWebEnginePage::Forward, this->mp_webView->isWebActionEnabled(QWebEnginePage::Forward));
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
                    auto pos = mp_tabBar->mapToGlobal(QPoint(-3, mp_tabBar->currentWebView()->height() - mp_tabBar->PdmHeightMM + 2));
                    QToolTip::showText(pos, link);
                }
            });
}

void ZimView::openFindInPageBar()
{
    mp_findInPageBar->show();
    mp_findInPageBar->getFindLineEdit()->setFocus();
}
