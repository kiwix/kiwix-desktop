#include "topwidget.h"

#include "kconstants.h"
#include "kiwixapp.h"

#include <QMouseEvent>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent),
    fullScreen(false)
{
    m_historyBackAction = new QAction(this);
    m_historyBackAction->setIcon(QIcon(":/icons/back.svg"));
    m_historyBackAction->setText("back");
    m_historyBackAction->setToolTip("back");
    connect(m_historyBackAction, &QAction::triggered, [this](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });
    addAction(m_historyBackAction);
    m_historyForwardAction = new QAction(this);
    m_historyForwardAction->setIcon(QIcon(":/icons/forward.svg"));
    m_historyForwardAction->setText("forward");
    m_historyForwardAction->setToolTip("forward");
    connect(m_historyForwardAction, &QAction::triggered, [this](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });
    addAction(m_historyForwardAction);
    addSeparator();

    addWidget(&searchEntry);

    addSeparator();

#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/minimize.svg"), "minimize", parent, SLOT(showMinimized()));
#endif
    fullScreenAction = addAction(QIcon(":/icons/full-screen-enter.svg"), "fullscreen", this, SLOT(toggleFullScreen()));
    normalScreenAction = addAction(QIcon(":/icons/full-screen-exit.svg"), "unfullscreen", this, SLOT(toggleFullScreen()));
    normalScreenAction->setVisible(false);
#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/close.svg"), "close", parent, SLOT(close()));
#endif
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

void TopWidget::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    switch (action) {
    case QWebEnginePage::Back:
        m_historyBackAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        m_historyForwardAction->setEnabled(enabled);
        break;
    default:
        break;
    }
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
