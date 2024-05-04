#include "contentmanagerdelegate.h"
#include <QtGui>
#include <QApplication>
#include <QDialog>
#include <QStyleOptionViewItem>
#include "kiwixapp.h"
#include <QStyleOptionViewItem>
#include "rownode.h"
#include "descriptionnode.h"
#include "portutils.h"

ContentManagerDelegate::ContentManagerDelegate(QObject *parent)
    : QStyledItemDelegate(parent), baseButton(new QPushButton)
{
    baseButton->setStyleSheet("background-color: white;"
                              "border: 0;"
                              "font-weight: bold;"
                              "font-family: Selawik;"
                              "color: blue;"
                              "margin: 4px;");
    QImage placeholderIconFile(":/icons/placeholder-icon.png");
    QBuffer buffer(&placeholderIcon);
    buffer.open(QIODevice::WriteOnly);
    placeholderIconFile.save(&buffer, "png");
}

namespace
{

void createPauseSymbol(QPainter *painter, const QRect& buttonRect)
{
    QPen pen;
    pen.setWidth(3);
    QPainterPath path;
    const int x = buttonRect.left() + 12.5;
    const int y = buttonRect.top() + 10;
    pen.setColor("#3366cc");
    path.moveTo(x, y);
    path.lineTo(x, y + 10);
    painter->strokePath(path, pen);
    path.moveTo(x + 5, y);
    path.lineTo(x + 5, y + 10);
    painter->strokePath(path, pen);
}

void createResumeSymbol(QPainter *painter, const QRect& buttonRect)
{
    QPen pen;
    pen.setWidth(3);
    QPainterPath path;
    const int x = buttonRect.left() + 12.5;
    const int y = buttonRect.top() + 8;
    pen.setColor("#3366cc");
    path.moveTo(x, y);
    path.lineTo(x, y + 15);
    path.lineTo(x + 10, y + 8);
    path.lineTo(x, y);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->strokePath(path, pen);
}

void createArc(QPainter *painter, int startAngle, int spanAngle, QRect rectangle, QPen pen)
{
    painter->setRenderHint(QPainter::Antialiasing);
    int arcX = rectangle.x();
    int arcY = rectangle.y();
    int arcW = rectangle.width();
    int arcH = rectangle.height();
    QPainterPath path;
    path.moveTo(arcX + arcW, arcY + arcH/2);
    path.arcTo(rectangle, startAngle, spanAngle);
    painter->strokePath(path, pen);
}

void createCancelButton(QPainter *painter, const QRect& r)
{
    QPen p;
    p.setWidth(3);
    p.setColor("#dd3333");
    createArc(painter, 0, 360, r, p);
    painter->setPen(p);
    auto oldFont = painter->font();
    auto bFont = oldFont;
    bFont.setBold(true);
    painter->setFont(bFont);
    painter->drawText(r, Qt::AlignCenter | Qt::AlignJustify, "X");
    painter->setFont(oldFont);
}

void createDownloadStats(QPainter *painter, QRect box, QString downloadSpeed, QString completedLength)
{
    QPen pen;
    int x = box.x();
    int y = box.y();
    int w = box.width();
    int h = box.height();
    pen.setColor("#666666");
    painter->setPen(pen);
    auto oldFont = painter->font();
    painter->setFont(QFont("Selawik", 8));
    QRect nRect(x - 10, y - 10, w, h);
    painter->drawText(nRect,Qt::AlignCenter | Qt::AlignJustify, downloadSpeed);
    QRect fRect(x - 10, y + 10, w, h);
    painter->drawText(fRect,Qt::AlignCenter | Qt::AlignJustify, completedLength);
    painter->setFont(oldFont);
}

struct DownloadControlLayout
{
    QRect pauseResumeButtonRect;
    QRect cancelButtonRect;
};

DownloadControlLayout getDownloadControlLayout(QRect box)
{
    const int x = box.left();
    const int y = box.top();
    const int w = box.width();
    const int h = box.height();

    const int buttonW = w - 90;
    const int buttonH = h - 40;

    DownloadControlLayout dcl;
    dcl.pauseResumeButtonRect = QRect(x + w/2 + 20, y + 20, buttonW, buttonH);
    dcl.cancelButtonRect      = QRect(x + w/2 - 20, y + 20, buttonW, buttonH);
    return dcl;
}

void showDownloadProgress(QPainter *painter, QRect box, const DownloadState& downloadInfo)
{
    const DownloadControlLayout dcl = getDownloadControlLayout(box);
    double progress  = (double) (downloadInfo.progress) / 100;
    progress = -progress;
    auto completedLength = downloadInfo.completedLength;
    auto downloadSpeed = downloadInfo.downloadSpeed;

    if (downloadInfo.paused) {
        createResumeSymbol(painter, dcl.pauseResumeButtonRect);
        createCancelButton(painter, dcl.cancelButtonRect);
    } else {
        createPauseSymbol(painter, dcl.pauseResumeButtonRect);
        createDownloadStats(painter, box, downloadSpeed, completedLength);
    }

    QPen pen;
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    pen.setColor("#eaecf0");
    createArc(painter, 0, 360, dcl.pauseResumeButtonRect, pen);

    int startAngle = 0;
    int spanAngle = progress * 360;
    pen.setColor("#3366cc");
    createArc(painter, startAngle, spanAngle, dcl.pauseResumeButtonRect, pen);
}

} // unnamed namespace

void ContentManagerDelegate::paintButton(QPainter *p, const QRect &r, QString t) const
{
    QStyleOptionButton button;
    button.rect = r;
    button.state = QStyle::State_Enabled;
    button.text = t;
    baseButton->style()->drawControl( QStyle::CE_PushButton, &button, p, baseButton.data());
}

void ContentManagerDelegate::paintBookState(QPainter *p, QRect r, const QModelIndex &index) const
{
    const auto node = static_cast<RowNode*>(index.internalPointer());
    const auto id = node->getBookId();
    switch ( KiwixApp::instance()->getContentManager()->getBookState(id) ) {
    case ContentManager::BookState::AVAILABLE_LOCALLY_AND_HEALTHY:
        return paintButton(p, r, gt("open"));

    case ContentManager::BookState::AVAILABLE_ONLINE:
        return paintButton(p, r, gt("download"));

    case ContentManager::BookState::DOWNLOADING:
    case ContentManager::BookState::DOWNLOAD_PAUSED:
    case ContentManager::BookState::DOWNLOAD_ERROR:
        return showDownloadProgress(p, r, *node->getDownloadState());

    default:
        return;
    }
}

void ContentManagerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect r = option.rect;
    if (index.parent().isValid()) {
        // additional info
        QRect nRect = r;
        auto viewWidth = KiwixApp::instance()->getContentManager()->getView()->getView()->width();
        nRect.setWidth(viewWidth);
        painter->drawText(nRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::UserRole+1).toString());
        return;
    }
    QStyleOptionViewItem eOpt = option;
    if (index.column() == 5) {
        paintBookState(painter, option.rect, index);
        return;
    }
    if (index.column() == 0) {
        auto iconData = index.data().value<QByteArray>();
        if (iconData.isNull())
            iconData = placeholderIcon;
        QPixmap pix;
        pix.loadFromData(iconData);
        QIcon icon(pix);
        icon.paint(painter, QRect(r.left()+10, r.top()+10, 30, 50));
        return;
    }
    if (index.column() == 1) {
        auto bFont = painter->font();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        bFont.setWeight(60);
#else
        bFont.setWeight(QFont::DemiBold);
#endif
        eOpt.font = bFont;
    }
    QStyledItemDelegate::paint(painter, eOpt, index);
}

bool ContentManagerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(model);

    if(event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent * e = (QMouseEvent *)event;
        int clickX = portutils::getX(*e);
        int clickY = portutils::getY(*e);

        QRect r = option.rect;
        int x,y,w,h;
        x = r.left();
        y = r.top();
        w = r.width();
        h = r.height();

        if (e->button() == Qt::MiddleButton && index.column() != 5) {
            KiwixApp::instance()->getContentManager()->openBookWithIndex(index);
            return true;
        }

        const auto lastColumnClicked = ((index.column() == 5) && (clickX > x && clickX < x + w)
                                                        && (clickY > y && clickY < y + h));

        if (lastColumnClicked)
            handleLastColumnClicked(index, e, option);
    }

    return true;
}

void ContentManagerDelegate::handleLastColumnClicked(const QModelIndex& index, QMouseEvent *mouseEvent, const QStyleOptionViewItem &option)
{
    const auto node = static_cast<RowNode*>(index.internalPointer());
    const auto id = node->getBookId();
    int clickX = portutils::getX(*mouseEvent);

    QRect r = option.rect;
    int x = r.left();
    int w = r.width();

    ContentManager& contentMgr = *KiwixApp::instance()->getContentManager();
    switch ( contentMgr.getBookState(id) ) {
    case ContentManager::BookState::AVAILABLE_LOCALLY_AND_HEALTHY:
        return contentMgr.openBook(id);

    case ContentManager::BookState::AVAILABLE_ONLINE:
        return contentMgr.downloadBook(id, index);

    case ContentManager::BookState::DOWNLOADING:
        return contentMgr.pauseBook(id, index);

    case ContentManager::BookState::DOWNLOAD_PAUSED:
        return clickX < (x + w/2)
             ? contentMgr.cancelBook(id)
             : contentMgr.resumeBook(id, index);

    default:
        return;
    }
}

QSize ContentManagerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.parent().isValid()) {
        return QSize(300, 70);
    }
    return QSize(50, 70);
}
