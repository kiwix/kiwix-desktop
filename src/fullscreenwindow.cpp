#include "fullscreenwindow.h"
#include "kiwixapp.h"
#include <QAction>

FullScreenWindow::FullScreenWindow(QWebEngineView *oldView, QWidget *parent)
    : QWidget(parent)
    , m_view(new QWebEngineView(this))
    , m_notification(new FullScreenNotification(this))
    , m_oldView(oldView)
    , m_oldGeometry(oldView->window()->geometry())
{
    m_view->stackUnder(m_notification);

    auto exitAction = new QAction(this);
    exitAction->setShortcut(QKeySequence::FullScreen);
    connect(exitAction, &QAction::triggered, [=]() {
        KiwixApp::instance()->getAction(KiwixApp::ToggleFullscreenAction)->trigger();
    });
    addAction(exitAction);

    m_view->setPage(m_oldView->page());
    setGeometry(m_oldGeometry);
    showFullScreen();
    m_oldView->window()->hide();

}

FullScreenWindow::~FullScreenWindow()
{
    m_oldView->setPage(m_view->page());
    m_oldView->window()->setGeometry(m_oldGeometry);
    m_oldView->window()->show();
    hide();
}

void FullScreenWindow::setWebEnginePage()
{
    m_oldView->setPage(m_view->page());
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