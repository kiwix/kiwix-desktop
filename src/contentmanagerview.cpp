#include "contentmanagerview.h"
#include <QFile>
#include "kiwixapp.h"
#include "contentmanagerdelegate.h"

ContentManagerView::ContentManagerView(QWidget *parent)
    : QTreeView(parent)
{
    setSortingEnabled(true);
    auto managerDelegate = new ContentManagerDelegate();
    setItemDelegate(managerDelegate);
    QFile file(QString::fromUtf8(":/css/_contentManager.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    setStyleSheet(styleSheet);
    setContextMenuPolicy(Qt::CustomContextMenu);
}
