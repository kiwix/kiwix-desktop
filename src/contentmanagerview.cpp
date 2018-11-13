#include "contentmanagerview.h"
#include <QFile>
#include <QWebEngineProfile>
#include "kiwixapp.h"

ContentManagerView::ContentManagerView(QWidget *parent)
    : QWebEngineView(parent)
{
    page()->setWebChannel(&m_webChannel);
    auto profile = page()->profile();
    auto app = KiwixApp::instance();
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
}


void ContentManagerView::registerObject(const QString& id, QObject* object)
{
    m_webChannel.registerObject(id, object);
}


void ContentManagerView::setHtml()
{
    QFile contentFile(":texts/_contentManager.html");
    contentFile.open(QIODevice::ReadOnly);
    auto byteContent = contentFile.readAll();
    contentFile.close();
    QWebEngineView::setHtml(byteContent);
}
