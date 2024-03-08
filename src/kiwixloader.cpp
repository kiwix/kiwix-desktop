#include "kiwixloader.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QSizePolicy>

KiwixLoader::KiwixLoader(QWidget *parent)
    : QWidget(parent), m_timer(nullptr)
{
    setFixedSize(parent->width(), parent->height());
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &KiwixLoader::updateAnimation);
}

KiwixLoader::~KiwixLoader()
{
}

void KiwixLoader::stopAnimation()
{
    m_timer->stop();
}

void KiwixLoader::startAnimation()
{
    m_timer->start(20);
}

void createArc(QPainter &painter, int startAngle, int spanAngle, QRect rectangle, QPen pen)
{
    painter.setRenderHint(QPainter::Antialiasing);
    int arcX = rectangle.x();
    int arcY = rectangle.y();
    int arcW = rectangle.width();
    int arcH = rectangle.height();
    QPainterPath path;
    path.moveTo(arcX + arcW, arcY + arcH/2);
    path.arcTo(rectangle, startAngle, spanAngle);
    painter.strokePath(path, pen);
}

void KiwixLoader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = 100;
    int height = 100;
    setFixedSize(this->parentWidget()->width(), this->parentWidget()->height());
    int centerX = this->parentWidget()->width()/2 - width;
    int centerY = this->parentWidget()->height()/2 - height;

    QPen pen;
    pen.setWidth(5);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rectangle(centerX, centerY, width, height);

    pen.setColor("#eaecf0");
    createArc(painter, 0, 360, rectangle, pen);

    int startAngle = 0;
    int spanAngle = -progress;
    pen.setColor("#3366cc");
    createArc(painter, startAngle, spanAngle, rectangle, pen);
}

void KiwixLoader::updateAnimation()
{
    progress += 10;
    if (progress == 360)
        progress = 0;
    update();
}
