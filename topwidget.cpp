#include "topwidget.h"
#include "kiwixapp.h"

#include <QMouseEvent>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent),
    fullScreen(false)
{
    addAction(QIcon(":/icons/back.svg"), "back");
    addAction(QIcon(":/icons/forward.svg"), "forward");
    addSeparator();

    addWidget(&searchEntry);

    addSeparator();

    addAction(QIcon(":/icons/minimize.svg"), "minimize", parent, SLOT(showMinimized()));
    fullScreenAction = addAction(QIcon(":/icons/full-screen-enter.svg"), "fullscreen", this, SLOT(toggleFullScreen()));
    normalScreenAction = addAction(QIcon(":/icons/full-screen-exit.svg"), "unfullscreen", this, SLOT(toggleFullScreen()));
    normalScreenAction->setVisible(false);
    addAction(QIcon(":/icons/close.svg"), "close", parent, SLOT(close()));
    setMovable(false);
}


void TopWidget::toggleFullScreen() {
    if (fullScreen)
        parentWidget()->showNormal();
    else
        parentWidget()->showFullScreen();
    fullScreen = !fullScreen;
    fullScreenAction->setVisible(!fullScreen);
    normalScreenAction->setVisible(fullScreen);
}


void TopWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() != Qt::LeftButton)
        return;

    m_pCursor = event->globalPos() + frameGeometry().topLeft() - parentWidget()->frameGeometry().topLeft();
    timestamp = event->timestamp();
    event->accept();
}

void TopWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->timestamp() <= timestamp)
        return;

    timestamp = event->timestamp();
    auto delta = event->globalPos() - m_pCursor;
    parentWidget()->move(delta);
    event->accept();
}
