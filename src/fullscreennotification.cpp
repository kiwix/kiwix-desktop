#include "fullscreennotification.h"
#include "kiwixapp.h"
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

FullScreenNotification::FullScreenNotification(QWidget *parent)
    : QLabel(parent)
    , m_animations(new QSequentialAnimationGroup(this))
    , m_effect(new QGraphicsOpacityEffect(this))
{
    setText(gt("fullscreen-notification"));
    setStyleSheet(
        "font-size: 24px;"
        "color: white;"
        "background-color: black;"
        "border-color: white;"
        "border-width: 2px;"
        "border-style: solid;"
        "padding: 100px");
    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_effect->setOpacity(1);
    setGraphicsEffect(m_effect);

    m_animations->addPause(3000);
    auto opacityAnimation = new QPropertyAnimation(m_effect, "opacity", m_animations);
    opacityAnimation->setDuration(2000);
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_animations->addAnimation(opacityAnimation);

    connect(m_animations, &QAbstractAnimation::finished,
            [this]{ hide(); });
}
void FullScreenNotification::show()
{
    m_effect->setOpacity(1);
    m_animations->start();
    QLabel::show();
}
