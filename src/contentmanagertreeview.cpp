#include "contentmanagertreeview.h"
#include "contentmanagermodel.h"
#include "kiwixapp.h"

#include <QCursor>
#include <QPainter>

ContentManagerTreeView::ContentManagerTreeView(QWidget *parent)
    : QTreeView(parent)
{
    viewport()->setMouseTracking(true);
}

bool ContentManagerTreeView::viewportEvent(QEvent *event)
{
    // The view only repaints the row under the mouse: repaint all when the
    // hovered book changes, so that its other row follows.
    const auto type = event->type();
    if (type == QEvent::MouseMove || type == QEvent::HoverMove
     || type == QEvent::Leave || type == QEvent::HoverLeave) {
        const bool inside = type == QEvent::MouseMove || type == QEvent::HoverMove;
        const QModelIndex book = inside
            ? bookIndex(indexAt(viewport()->mapFromGlobal(QCursor::pos())))
            : QModelIndex();
        if (m_hoveredBook != book) {
            m_hoveredBook = book;
            viewport()->update();
        }
    }
    return QTreeView::viewportEvent(event);
}

void ContentManagerTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // The branch area (left of the first column) isn't painted by the
    // delegate: highlight it like the rest of the item.
    const auto book = bookIndex(index);
    const bool selected = selectionModel() && selectionModel()->isSelected(book);
    if (selected || book == m_hoveredBook)
        painter->fillRect(rect, themeColor(selected ? "ACCENT_BACKGROUND" : "HOVER_BACKGROUND"));
    QTreeView::drawBranches(painter, rect, index);
}

void ContentManagerTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    // A book's description is drawn selected with it.
    viewport()->update();
}
