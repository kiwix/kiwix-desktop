#include "contentmanagerheader.h"
#include <QPainter>
#include "kiwixapp.h"

ContentManagerHeader::ContentManagerHeader(Qt::Orientation orientation, QWidget *parent)
 : QHeaderView(orientation, parent)
{}

ContentManagerHeader::~ContentManagerHeader()
{}

void ContentManagerHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    // This is required so that the sort indicator icon is not shown in first column
    if (logicalIndex == 0) {
        painter->save();
        const bool dark = KiwixApp::instance() && KiwixApp::instance()->isDarkTheme();
        painter->fillRect(rect, dark ? QColor("#202124") : QColor("#ffffff"));
        painter->restore();
        return;
    }
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
}

