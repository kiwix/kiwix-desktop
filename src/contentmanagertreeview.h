#ifndef CONTENTMANAGERTREEVIEW_H
#define CONTENTMANAGERTREEVIEW_H

#include <QTreeView>
#include <QPersistentModelIndex>

// The book list. A book and its description are shown as one item: they are
// hovered and selected together (descriptions can't be selected themselves,
// see ContentManagerDelegate::paint() for their cells).
class ContentManagerTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit ContentManagerTreeView(QWidget *parent = nullptr);

    // The book under the mouse (its first column), if any.
    QModelIndex hoveredBook() const { return m_hoveredBook; }

protected:
    bool viewportEvent(QEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    QPersistentModelIndex m_hoveredBook;
};

#endif // CONTENTMANAGERTREEVIEW_H
