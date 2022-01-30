#include "fullscreennotification.h"
#include "kiwixapp.h"
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

FullScreenNotification::FullScreenNotification(QWidget *parent)
    : QLabel(parent)
    , m_previouslyVisible(false)
{
    auto text = gt("fullscreen-notification");
    text = text.replace("{{FullScreenKey}}", QKeySequence(QKeySequence::FullScreen).toString());
    setText(text);
    setStyleSheet(
        "font-size: 24px;"
        "color: white;"
        "background-color: black;"
        "border-color: white;"
        "border-width: 2px;"
        "border-style: solid;"
        "padding: 100px");
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto effect = new QGraphicsOpacityEffect;
    effect->setOpacity(1);
    setGraphicsEffect(effect);

    auto animations = new QSequentialAnimationGroup(this);
    animations->addPause(3000);
    auto opacityAnimation = new QPropertyAnimation(effect, "opacity", animations);
    opacityAnimation->setDuration(2000);
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    animations->addAnimation(opacityAnimation);

    connect(this, &FullScreenNotification::shown,
            [animations](){ animations->start(); });

    connect(animations, &QAbstractAnimation::finished,
            [this](){ this->hide(); });
}

void FullScreenNotification::showEvent(QShowEvent *event)
{
    QLabel::showEvent(event);
    if (!m_previouslyVisible && isVisible())
        emit shown();
    m_previouslyVisible = isVisible();
}