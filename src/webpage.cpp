#include "webpage.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include "kiwixapp.h"
#include <QWebEngineProfile>
#include <QWebChannel>
#include <QWebEngineScript>

#include "kiwixwebchannelobject.h"

WebPage::WebPage(QObject *parent) :
    QWebEnginePage(KiwixApp::instance()->getProfile(), parent)
{
    action(QWebEnginePage::SavePage)->setVisible(false);
    action(QWebEnginePage::ViewSource)->setVisible(false);
    action(QWebEnginePage::Reload)->setVisible(false);

    QWebChannel *channel = new QWebChannel(this);
    KiwixWebChannelObject *kiwixChannelObj = new KiwixWebChannelObject(this);
    setWebChannel(channel, QWebEngineScript::UserWorld);
    channel->registerObject("kiwixChannelObj", kiwixChannelObj);

    auto app = KiwixApp::instance();
    auto tocAction = app->getAction(KiwixApp::ToggleTOCAction);
    auto readingListAction = app->getAction(KiwixApp::ToggleReadingListAction);
    connect(tocAction, &QAction::toggled,
            kiwixChannelObj, &KiwixWebChannelObject::tocVisibleChanged);
    connect(readingListAction, &QAction::toggled, this, [=](bool visible){
        if (visible && tocAction->isChecked())
            tocAction->toggle();
    });
    connect(tocAction, &QAction::toggled, this, [=](bool visible){
        if (visible && readingListAction->isChecked())
            readingListAction->toggle();
    });
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/)
{
    if (url.scheme() != "zim") {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
