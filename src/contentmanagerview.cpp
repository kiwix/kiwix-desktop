#include "kiwixapp.h"
#include "contentmanagerview.h"
#include <QFile>
#include <QWebEngineProfile>

ContentManagerView::ContentManagerView(QWidget *parent)
    : QWebEngineView(parent)
{
    QWebEnginePage* page = new QWebEnginePage(KiwixApp::instance()->getProfile(), this);
    setPage(page);
    page->setWebChannel(&m_webChannel);
    setContextMenuPolicy( Qt::NoContextMenu );
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
