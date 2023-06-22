#include <QtGui>
#include "contentmanagerdelegate.h"
#include <QApplication>
#include <QDialog>
#include <QStyleOptionViewItemV4>
#include "kiwixapp.h"
#include <QStyleOptionViewItem>

ContentManagerDelegate::ContentManagerDelegate(QObject *parent)
    : QStyledItemDelegate(parent), baseButton(new QPushButton)
{
    baseButton->setStyleSheet("background-color: white;"
                              "border: 0;"
                              "font-weight: bold;"
                              "font-family: Selawik;"
                              "color: blue;"
                              "margin: 4px;");
}


void ContentManagerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton button;
    QRect r = option.rect;
    int x,y,w,h;
    x = r.left();
    y = r.top();
    w = r.width();
    h = r.height();
    button.rect = QRect(x,y,w,h);
    button.text = "Open";
    button.state = QStyle::State_Enabled;
    QStyleOptionViewItem eOpt = option;
    if (index.data(Qt::UserRole+1) != QVariant()) {
        // additional info role
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    if (index.column() == 5) {
        baseButton->style()->drawControl( QStyle::CE_PushButton, &button, painter, baseButton.data());
        return;
    }
    if (index.column() == 0) {
        const auto icon = index.data().value<QIcon>();
        icon.paint(painter, QRect(x+10, y+10, 30, 50));
        return;
    }
    if (index.column() == 1) {
        auto bFont = painter->font();
        bFont.setBold(true);
        eOpt.font = bFont;
    }
    QStyledItemDelegate::paint(painter, eOpt, index);
}

bool ContentManagerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent * e = (QMouseEvent *)event;
        int clickX = e->x();
        int clickY = e->y();

        QRect r = option.rect;//getting the rect of the cell
        int x,y,w,h;
        x = r.left();//the X coordinate
        y = r.top();//the Y coordinate
        w = r.width();//button width
        h = r.height();//button height


        if(index.column() == 5 && clickX > x && clickX < x + w )
            if( clickY > y && clickY < y + h )
            {
                const auto modeel = index.model();
                const auto id = modeel->data(index).toString();
                KiwixApp::instance()->getContentManager()->openBook(id);
            }
    }

    return true;
}

QSize ContentManagerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::UserRole+1) != QVariant()) {
        return QSize(300, 70);
    }
    return QSize(50, 70);
}
