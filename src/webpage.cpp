#include "webpage.h"

#include <QDesktopServices>

WebPage::WebPage(QObject *parent) :
    QWebEnginePage(parent)
{

}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (url.scheme() != "zim") {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;

}
