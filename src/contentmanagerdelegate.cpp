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
#include "css_constants.h"

ContentManagerDelegate::ContentManagerDelegate(QObject *parent)
    : QStyledItemDelegate(parent), baseButton(new QPushButton)
{
    baseButton->setStyleSheet("background-color: #00000000;"
                              "border: 0;"
                              "font-weight: bold;"
                              "font-family: Selawik;"
                              "color: blue;"
                              "margin: 4px;");
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

// Modified to draw three lines: download speed, completed length and estimated time remaining.
void createDownloadStats(QPainter *painter, QRect box, QString downloadSpeed, QString completedLength, QString timeRemaining)
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
    QRect speedRect(x - 20, y - 10, w, h);
    painter->drawText(speedRect, Qt::AlignCenter | Qt::AlignJustify, downloadSpeed);
    QRect completedRect(x - 20, y + 10, w, h);
    painter->drawText(completedRect, Qt::AlignCenter | Qt::AlignJustify, completedLength);
    QRect timeRect(x - 20, y + 30, w, h);
    painter->drawText(timeRect, Qt::AlignCenter | Qt::AlignJustify, timeRemaining);
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
    dcl.pauseResumeButtonRect = QRect(x + w/2 + 10, y + 20, buttonW, buttonH);
    dcl.cancelButtonRect      = QRect(x + w/2 - 40, y + 20, buttonW, buttonH);
    return dcl;
}

void showDownloadProgress(QPainter *painter, QRect box, const DownloadState& downloadInfo)
{
    const DownloadControlLayout dcl = getDownloadControlLayout(box);
    double progress  = (double) (downloadInfo.progress) / 100;
    progress = -progress;
    auto completedLength = downloadInfo.completedLength;
    auto downloadSpeed = downloadInfo.getDownloadSpeed();
    auto timeRemaining = downloadInfo.getEstimatedTimeRemaining();

    if (downloadInfo.getStatus() == DownloadState::PAUSED) {
        createResumeSymbol(painter, dcl.pauseResumeButtonRect);
        createCancelButton(painter, dcl.cancelButtonRect);
    } else if (downloadInfo.getStatus() == DownloadState::DOWNLOADING) {
        createPauseSymbol(painter, dcl.pauseResumeButtonRect);
        // Pass the estimated time remaining to display it
        createDownloadStats(painter, box, downloadSpeed, completedLength, timeRemaining);
    }

    QPen pen;
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    pen.setColor("#dadce0");
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

void ContentManagerDelegate::paintBookState(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    const QRect r = opt.rect;
    if (opt.state & QStyle::State_MouseOver) {
        // don't paint over the line separator
        const auto cellInternalArea = r.adjusted(0, 0, 0, -1);
        p->fillRect(cellInternalArea, QBrush("#eaecf0"));
    }
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
    if (isDescriptionIndex(index))
        return QStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem eOpt = option;
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

    if (index.column() == 5) {
        paintBookState(painter, eOpt, index);
    }
}

bool ContentManagerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(model);

    if(event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent * e = (QMouseEvent *)event;
        if (e->button() == Qt::MiddleButton && index.column() != 5) {
            KiwixApp::instance()->getContentManager()->openBookWithIndex(index);
            return true;
        }

        const QPoint clickPoint(portutils::getX(*e), portutils::getY(*e));

        if ( index.column() == 5 && option.rect.contains(clickPoint, true) )
            handleLastColumnClicked(index, e, option);
    }

    return true;
}

void ContentManagerDelegate::handleLastColumnClicked(const QModelIndex& index, QMouseEvent *mouseEvent, const QStyleOptionViewItem &option)
{
    const auto node = static_cast<RowNode*>(index.internalPointer());
    const auto id = node->getBookId();
    const auto downloadState = node->getDownloadState();

    const int clickX = portutils::getX(*mouseEvent);
    const int clickY = portutils::getY(*mouseEvent);
    const QPoint clickPoint(clickX, clickY);
    const DownloadControlLayout dcl = getDownloadControlLayout(option.rect);

    ContentManager& contentMgr = *KiwixApp::instance()->getContentManager();
    switch ( contentMgr.getBookState(id) ) {
    case ContentManager::BookState::AVAILABLE_LOCALLY_AND_HEALTHY:
        return contentMgr.openBook(id);

    case ContentManager::BookState::AVAILABLE_ONLINE:
        return contentMgr.downloadBook(id);

    case ContentManager::BookState::DOWNLOADING:
        if ( downloadState->getStatus() == DownloadState::DOWNLOADING ) {
            if ( dcl.pauseResumeButtonRect.contains(clickPoint) ) {
                contentMgr.pauseBook(id, index);
            }
        }
        return;

    case ContentManager::BookState::DOWNLOAD_PAUSED:
        if ( downloadState->getStatus() == DownloadState::PAUSED ) {
            if ( dcl.cancelButtonRect.contains(clickPoint) ) {
                 contentMgr.cancelBook(id);
            } else if ( dcl.pauseResumeButtonRect.contains(clickPoint) ) {
                 contentMgr.resumeBook(id, index);
            }
        }
        return;

    default:
        return;
    }
}

QSize ContentManagerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (isDescriptionIndex(index))
    {
        const auto treeView = KiwixApp::instance()->getContentManager()->getView()->getView();

        const int width = treeView->header()->length() - 2*treeView->indentation();
        const int verticalPadding = CSS::ContentManagerCSS::QTreeView::padding;
        const int horizontalPadding = CSS::ContentManagerCSS::QTreeView::padding;
        QRect descRect(0, 0, width - 2 * horizontalPadding, 0);

        /* Based on the rectangle and text, find the best fitting size. */
        QFontMetrics fm(option.font);
        const QString text = index.data().toString();
        const auto format = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap;
        const int textHeight = fm.boundingRect(descRect, format, text).height();
        return QSize(width, std::max(textHeight + verticalPadding, 70));
    }
    return QSize(50, 70);
}
