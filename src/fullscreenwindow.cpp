#include "fullscreenwindow.h"
#include "kiwixapp.h"
#include <QAction>

FullScreenWindow::FullScreenWindow()
    : m_view(new QWebEngineView(this))
    , m_notification(new FullScreenNotification(this))
{
    m_view->stackUnder(m_notification);
    setMouseTracking(true);
    auto exitAction = new QAction(this);
    exitAction->setShortcut(Qt::Key_Escape);
    connect(exitAction, &QAction::triggered, []{
        KiwixApp::instance()->getAction(KiwixApp::ToggleFullscreenAction)->trigger();
    });
    addAction(exitAction);
}

void FullScreenWindow::reset(QWebEngineView *oldView)
{
    m_oldView = oldView;
    m_view->setPage(m_oldView->page());
    showFullScreen();
    m_notification->show();
}
void FullScreenWindow::exit()
{
    hide();
    m_oldView->setPage(m_view->page());
    m_oldView->window()->show();
    m_oldView = nullptr;
}
bool FullScreenWindow::isFullScreen()
{
    return m_oldView;
}
void FullScreenWindow::resizeEvent(QResizeEvent *event)
{
    QRect viewGeometry(QPoint(0, 0), size());
    m_view->setGeometry(viewGeometry);

    QRect notificationGeometry(QPoint(0, 0), m_notification->sizeHint());
    notificationGeometry.moveCenter(viewGeometry.center());
    m_notification->setGeometry(notificationGeometry);

    QWidget::resizeEvent(event);
}
