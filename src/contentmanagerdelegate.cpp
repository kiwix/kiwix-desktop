#include <QtGui>
#include "contentmanagerdelegate.h"
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

void createPauseSymbol(QPainter *painter, int x, int y)
{
    QPen pen;
    pen.setWidth(3);
    QPainterPath path;
    x += 12.5;
    y += 10;
    pen.setColor("#3366cc");
    path.moveTo(x, y);
    path.lineTo(x, y + 10);
    painter->strokePath(path, pen);
    path.moveTo(x + 5, y);
    path.lineTo(x + 5, y + 10);
    painter->strokePath(path, pen);
}

void createResumeSymbol(QPainter *painter, int x, int y)
{
    QPen pen;
    pen.setWidth(3);
    QPainterPath path;
    x += 12.5;
    y += 8;
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

void createCancelSymbol(QPainter *painter, int x, int y, int w, int h)
{
    QPen p;
    p.setWidth(3);
    p.setColor("#dd3333");
    QRect r(x, y, w, h);
    createArc(painter, 0, 360, r, p);
    painter->setPen(p);
    QRect nRect(x, y, w, h);
    auto oldFont = painter->font();
    auto bFont = oldFont;
    bFont.setBold(true);
    painter->setFont(bFont);
    painter->drawText(nRect, Qt::AlignCenter | Qt::AlignJustify, "X");
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

void showDownloadProgress(QPainter *painter, QRect box, const DownloadState& downloadInfo)
{
    int x,y,w,h;
    x = box.left();
    y = box.top();
    w = box.width();
    h = box.height();

    int arcX = x + w/2 + 20;
    int arcY = y + 20;
    int arcW = w - 90;
    int arcH = h - 40;

    double progress  = (double) (downloadInfo.progress) / 100;
    progress = -progress;
    auto completedLength = downloadInfo.completedLength;
    auto downloadSpeed = downloadInfo.downloadSpeed;

    if (downloadInfo.paused) {
        createResumeSymbol(painter, arcX, arcY);
        createCancelSymbol(painter, x + w/2 - 20, arcY, arcW, arcH);
    } else {
        createPauseSymbol(painter, arcX, arcY);
        createDownloadStats(painter, box, downloadSpeed, completedLength);
    }

    QPen pen;
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rectangle(arcX, arcY, arcW, arcH);

    pen.setColor("#eaecf0");
    createArc(painter, 0, 360, rectangle, pen);

    int startAngle = 0;
    int spanAngle = progress * 360;
    pen.setColor("#3366cc");
    createArc(painter, startAngle, spanAngle, rectangle, pen);
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
    button.state = QStyle::State_Enabled;
    if (index.parent().isValid()) {
        // additional info
        QRect nRect = r;
        auto viewWidth = KiwixApp::instance()->getContentManager()->getView()->getView()->width();
        nRect.setWidth(viewWidth);
        painter->drawText(nRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::UserRole+1).toString());
        return;
    }
    auto node = static_cast<RowNode*>(index.internalPointer());
    try {
        const auto id = node->getBookId();
        const auto book = KiwixApp::instance()->getLibrary()->getBookById(id);
        if(KiwixApp::instance()->getContentManager()->getBookInfos(id, {"downloadId"})["downloadId"] != "") {
        } else {
            button.text = gt("open");
        }
    } catch (std::out_of_range& e) {
        button.text = gt("download");
    }
    QStyleOptionViewItem eOpt = option;
    if (index.column() == 5) {
        if (const auto downloadState = node->getDownloadState()) {
            showDownloadProgress(painter, r, *downloadState);
        }
        else {
            baseButton->style()->drawControl( QStyle::CE_PushButton, &button, painter, baseButton.data());
        }
        return;
    }
    if (index.column() == 0) {
        auto iconData = index.data().value<QByteArray>();
        if (iconData.isNull())
            iconData = placeholderIcon;
        QPixmap pix;
        pix.loadFromData(iconData);
        QIcon icon(pix);
        icon.paint(painter, QRect(x+10, y+10, 30, 50));
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

    if (const auto downloadState = node->getDownloadState()) {
        if (downloadState->paused) {
            if (clickX < (x + w/2)) {
                KiwixApp::instance()->getContentManager()->cancelBook(id, index);
            } else {
                KiwixApp::instance()->getContentManager()->resumeBook(id, index);
            }
        } else {
            KiwixApp::instance()->getContentManager()->pauseBook(id, index);
        }
    } else {
            try {
                const auto book = KiwixApp::instance()->getLibrary()->getBookById(id);
                KiwixApp::instance()->getContentManager()->openBook(id);
            } catch (std::out_of_range& e) {
                KiwixApp::instance()->getContentManager()->downloadBook(id, index);
            }
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
