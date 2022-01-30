#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include <QWidget>
#include <QWebEngineView>
#include "fullscreennotification.h"

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

class FullScreenNotification;

class FullScreenWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FullScreenWindow(QWebEngineView *oldView, QWidget *parent = nullptr);
    ~FullScreenWindow();
    void setWebEnginePage();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QWebEngineView *m_view;
    FullScreenNotification *m_notification;
    QWebEngineView *m_oldView;
    QRect m_oldGeometry;
};

#endif // FULLSCREENWINDOW_H
