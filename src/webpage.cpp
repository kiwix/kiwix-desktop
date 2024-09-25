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
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/)
{
    if (url.scheme() != "zim") {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
