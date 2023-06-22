#include "contentmanagerview.h"
#include <QFile>
#include "kiwixapp.h"

ContentManagerView::ContentManagerView(QWidget *parent)
    : QTreeView(parent)
{
    setSortingEnabled(true);
}
