#include "suggestionlistdelegate.h"
#include "kiwixapp.h"

#include <QPainter>

void SuggestionListDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    /* Paint without text and icon */
    QStyleOptionViewItem opt(option);
    QStyledItemDelegate::paint(painter, opt, QModelIndex());

    paintIcon(painter, opt, index);
    paintText(painter, opt, index);
}

void SuggestionListDelegate::paintIcon(QPainter *p,
                                       const QStyleOptionViewItem &opt,
                                       const QModelIndex &index) const
{
    QRect pixmapRect = opt.rect;

    /* See line-height&padding resources/css/popup.css QHeaderView::section. 
       We need to add 10px of padding to match header.
    */
    QSize mapSize = QSize(24, 24);
    auto pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(mapSize);
    if (KiwixApp::isRightToLeft())
    {
        int rightEnd = pixmapRect.width() - mapSize.width();
        pixmapRect.setX(pixmapRect.x() + rightEnd - 10);
    }
    else
        pixmapRect.setX(pixmapRect.x() + 10);

    pixmapRect.setY(pixmapRect.y() + (pixmapRect.height() - mapSize.height()) / 2);
    pixmapRect.setSize(mapSize);
    p->drawPixmap(pixmapRect, pixmap);
}

void SuggestionListDelegate::paintText(QPainter *p,
                                       const QStyleOptionViewItem &opt,
                                       const QModelIndex &index) const
{
    auto& searchBar = KiwixApp::instance()->getSearchBar();
    auto lineEditGeo = searchBar.getLineEdit().geometry();

    /* See SearchBar border in resources/css/style.css. Align text with text in
       line edit. Remove border from left() as completer is inside border.
    */
    auto left = lineEditGeo.left() - 1;
    QRect textRect = opt.rect;
    if (KiwixApp::isRightToLeft())
    {
        /* Calculate the distance of right side of search bar to line edit as
           text now starts on the right.
        */
        auto searchGeo = searchBar.geometry();
        auto right = searchGeo.width() - left - lineEditGeo.width();
        textRect.setWidth(textRect.width() - right);
    }
    else
        textRect.setX(textRect.x() + left);
    
    int flag = {Qt::AlignVCenter | Qt::AlignLeading};
    QString text = index.data(Qt::DisplayRole).toString();
    p->drawText(textRect, flag, text);
    return;
}
