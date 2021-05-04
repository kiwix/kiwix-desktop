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
    static FullScreenWindow &instance() {
        static FullScreenWindow singleton;
        return singleton;
    }
    void reset(QWebEngineView *oldView);
    void exit();
    bool isFullScreen();
protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    FullScreenWindow();
    QWebEngineView *m_view {nullptr};
    FullScreenNotification *m_notification {nullptr};
    QWebEngineView *m_oldView {nullptr};
};

#endif // FULLSCREENWINDOW_H
