#include "kiwixapp.h"
#include "settingsmanagerview.h"
#include <QFile>

SettingsManagerView::SettingsManagerView(QWidget *parent) : QWebEngineView(parent)
{
    page()->setWebChannel(&m_webChannel);
    setContextMenuPolicy( Qt::NoContextMenu );
}

void SettingsManagerView::registerObject(const QString& id, QObject* object)
{
    m_webChannel.registerObject(id, object);
}

void SettingsManagerView::setHtml()
{
    QFile contentFile(":texts/_settingsManager.html");
    contentFile.open(QIODevice::ReadOnly);
    auto byteContent = contentFile.readAll();
    contentFile.close();
    QWebEngineView::setHtml(byteContent);
}
