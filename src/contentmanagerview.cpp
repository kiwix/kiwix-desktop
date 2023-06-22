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
}
