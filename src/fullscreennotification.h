#ifndef FULLSCREENNOTIFICATION_H
#define FULLSCREENNOTIFICATION_H

#include <QLabel>

class FullScreenNotification : public QLabel
{
    Q_OBJECT
public:
    FullScreenNotification(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void shown();

private:
    bool m_previouslyVisible;
};

#endif // FULLSCREENNOTIFICATION_H