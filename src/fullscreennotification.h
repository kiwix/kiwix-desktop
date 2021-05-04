#ifndef FULLSCREENNOTIFICATION_H
#define FULLSCREENNOTIFICATION_H

#include <QLabel>
class QSequentialAnimationGroup;
class QGraphicsOpacityEffect;
class FullScreenNotification : public QLabel
{
    Q_OBJECT
public:
    FullScreenNotification(QWidget *parent = nullptr);
    void show();

private:
    QSequentialAnimationGroup *m_animations {nullptr};
    QGraphicsOpacityEffect *m_effect {nullptr};
};

#endif // FULLSCREENNOTIFICATION_H
