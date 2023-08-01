#include "contentmanagerheader.h"
#include <QPainter>

ContentManagerHeader::ContentManagerHeader(Qt::Orientation orientation, QWidget *parent)
 : QHeaderView(orientation, parent)
{}

ContentManagerHeader::~ContentManagerHeader()
{}

void ContentManagerHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    // This is required so that the sort indicator icon is not shown in first column
    if (logicalIndex == 0)
        return;
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
}

