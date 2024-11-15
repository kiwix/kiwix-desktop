#include "zimview.h"
#include "kiwixapp.h"
#include <QAction>
#include <QVBoxLayout>
#include <QToolTip>

#if defined(QT_TEXTTOSPEECH_LIB)
#include "texttospeechbar.h"
#endif

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
    setLayout(layout); // now 'mp_webView' has 'this' as the parent QObject
    mp_findInPageBar->hide();
    auto app = KiwixApp::instance();

#if defined(QT_TEXTTOSPEECH_LIB)
    mp_ttsBar = new TextToSpeechBar(this);
    layout->addWidget(mp_ttsBar);
    mp_ttsBar->hide();
    connect(mp_webView, &WebView::zimIdChanged, this, &ZimView::setSpeechLocaleByZimId);
    connect(app->getAction(KiwixApp::ReadArticleAction), &QAction::triggered, this, &ZimView::readArticle);
    connect(app->getAction(KiwixApp::ReadTextAction), &QAction::triggered, this, &ZimView::readSelectedText);
#endif

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

#if defined(QT_TEXTTOSPEECH_LIB)
void ZimView::readArticle()
{
    if (mp_tabBar->currentZimView() != this)
        return;

    mp_webView->page()->toPlainText([=](const QString& articleText){
        mp_ttsBar->speak(articleText);
        mp_ttsBar->speechShow();
    });
}

void ZimView::readSelectedText()
{
    if (mp_tabBar->currentZimView() != this || !mp_webView->page()->hasSelection())
        return;

    mp_ttsBar->speak(mp_webView->page()->selectedText());
    mp_ttsBar->speechShow();
}

void ZimView::setSpeechLocaleByZimId(const QString& zimId)
{
    try
    {
        const auto book = KiwixApp::instance()->getLibrary()->getBookById(zimId);
        const auto iso3 = QString::fromStdString(book.getLanguages().at(0));
        const auto iso2 = iso3.chopped(1);

        /* Try both 3 letter and 2 letter name. */
        const auto iso2Locale = QLocale(iso2);
        const bool isValidISO2 = iso2Locale.language() != QLocale::C;
        mp_ttsBar->setLocale(isValidISO2 ? iso2Locale : QLocale(iso3));
    } catch (...) { /* Blank */ }
}
#endif
