#include "suggestionlistdelegate.h"
#include "kiwixapp.h"
#include "css_constants.h"

#include <QPainter>

namespace HeaderSectionCSS = CSS::PopupCSS::QHeaderView::section;

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
    const int lineHeight = HeaderSectionCSS::lineHeight;
    const int paddingLeft = HeaderSectionCSS::paddingLeft;

    const QSize mapSize = QSize(lineHeight, lineHeight);
    auto pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(mapSize);

    /* Align icon to Header text */
    if (KiwixApp::isRightToLeft())
    {
        const int rightEnd = pixmapRect.width() - mapSize.width();
        pixmapRect.setX(pixmapRect.x() + rightEnd - paddingLeft);
    }
    else
        pixmapRect.setX(pixmapRect.x() + paddingLeft);

    /* Align middle */
    pixmapRect.setY(pixmapRect.y() + (pixmapRect.height() - mapSize.height()) / 2);
    pixmapRect.setSize(mapSize);
    p->drawPixmap(pixmapRect, pixmap);
}

void SuggestionListDelegate::paintText(QPainter *p,
                                       const QStyleOptionViewItem &opt,
                                       const QModelIndex &index) const
{
    auto& searchBar = KiwixApp::instance()->getSearchBar();
    const auto& lineEditGeo = searchBar.getLineEdit().geometry();

    /* Remove border from left() since left is is with respect to border. Detail
       reason on how this calculation comes about can be seen in
       SearchBarLineEdit::getCompleterRect();
    */
    const int left = lineEditGeo.left() - CSS::SearchBar::border;
    QRect textRect = opt.rect;
    if (KiwixApp::isRightToLeft())
    {
        const auto& searchGeo = searchBar.geometry();
        const int right = searchGeo.width() - left - lineEditGeo.width();
        textRect.setWidth(textRect.width() - right);
    }
    else
        textRect.setX(textRect.x() + left);
    
    const int flag = {Qt::AlignVCenter | Qt::AlignLeading};
    const QString text = index.data(Qt::DisplayRole).toString();

    /* Custom text elide. */
    const QFontMetrics metrics = opt.fontMetrics;
    const int elideMarkerLength = metrics.tightBoundingRect("(...)").width();
    const int textLength = textRect.width() - elideMarkerLength;
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, textLength);
    if (elidedText != text)
    {
        /* Remove built-in elide marker */
        elidedText.chop(1);

        /* drawText's Align direction determines text direction */
        const bool textDirFlipped = KiwixApp::isRightToLeft() != text.isRightToLeft();
        elidedText = textDirFlipped ? "(...)" + elidedText.trimmed() 
                                    : elidedText.trimmed() + "(...)";
        p->drawText(textRect, flag, elidedText);
    }
    else
        p->drawText(textRect, flag, text);
}
