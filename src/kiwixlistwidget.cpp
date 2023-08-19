#include "kiwixlistwidget.h"
#include <QKeyEvent>
#include <QToolTip>
#include <QCursor>
#include <QDebug>
#include "kiwixapp.h"
#include "klistwidgetitem.h"

KiwixListWidget::KiwixListWidget(QWidget *parent)
    : QListWidget(parent), currRow(0), m_visibleItems(count())
{
    connect(this, &KiwixListWidget::currRowChanged, this, &KiwixListWidget::handleCurrRowChange);
    setMouseTracking(true);
}

void KiwixListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        selectCurrent(item(m_mouseIndex));
    }
}

void KiwixListWidget::mouseMoveEvent(QMouseEvent *event)
{
    int oldRow = currRow;
    m_mouseIndex = row(itemAt(event->pos()));
    currRow = m_mouseIndex;
    emit(currRowChanged(oldRow, currRow));
}

void KiwixListWidget::hideEvent(QHideEvent *event)
{
    auto currItem = dynamic_cast<KListWidgetItem*>(item(currRow));
    if (currItem)
        currItem->disableHighlight();
    QListWidget::hideEvent(event);
}

void KiwixListWidget::resizeEvent(QResizeEvent *e)
{
    int oldRow = currRow;
    for (auto i = 0; i < count(); i++) {
        auto itemAtRow = item(i);
        if (!itemAtRow->isHidden() && !itemAtRow->isSelected()) {
            currRow = i;
            break;
        }
    }
    emit(currRowChanged(oldRow, currRow));
    QListWidget::resizeEvent(e);
}

void KiwixListWidget::handleCurrRowChange(int oldRow, int newRow)
{
    auto prevItem = dynamic_cast<KListWidgetItem*>(item(oldRow));
    if (prevItem) {
        prevItem->disableHighlight();
    }
    auto currItem = dynamic_cast<KListWidgetItem*>(item(newRow));
    if (currItem) {
        currItem->enableHighlight();
        scrollToItem(currItem, QAbstractItemView::EnsureVisible);
    }
    emit(dataChanged(QModelIndex(), QModelIndex()));
}

void KiwixListWidget::moveUp()
{
    if (selectedItems().size() == count())
        return;
    KListWidgetItem *currItem = dynamic_cast<KListWidgetItem*>(item(currRow));
    int oldRow = currRow;
    do {
        currRow--;
        if (currRow < 0) currRow = count() - 1;
        currItem = dynamic_cast<KListWidgetItem*>(item(currRow));
    } while (currItem->isSelected() || currItem->isHidden());
    emit(currRowChanged(oldRow, currRow));
}

void KiwixListWidget::moveDown()
{
    if (selectedItems().size() == count())
        return;
    KListWidgetItem *currItem = dynamic_cast<KListWidgetItem*>(item(currRow));
    int oldRow = currRow;
    do {
        currRow++;
        if (currRow == count()) currRow = 0;
        currItem = dynamic_cast<KListWidgetItem*>(item(currRow));
    } while (currItem->isSelected() || currItem->isHidden());
    emit(currRowChanged(oldRow, currRow));
}

void KiwixListWidget::selectCurrent()
{
    selectCurrent(item(currRow));
}

void KiwixListWidget::selectCurrent(QListWidgetItem *item)
{
    auto currItem = dynamic_cast<KListWidgetItem*>(item);
    if (currItem && !currItem->isSelected() && !currItem->isHidden()) {
        currItem->disableHighlight();
        currItem->setSelected(!currItem->isSelected());
        emit(itemPressed(currItem));
    }
}
