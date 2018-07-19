#include "topwidget.h"

#include "kconstants.h"
#include "kiwixapp.h"

#include <QMouseEvent>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent),
    m_fullScreen(false)
{
    mp_historyBackAction = new QAction(this);
    mp_historyBackAction->setIcon(QIcon(":/icons/back.svg"));
    mp_historyBackAction->setText("back");
    mp_historyBackAction->setToolTip("back");
    connect(mp_historyBackAction, &QAction::triggered, [this](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });
    addAction(mp_historyBackAction);
    mp_historyForwardAction = new QAction(this);
    mp_historyForwardAction->setIcon(QIcon(":/icons/forward.svg"));
    mp_historyForwardAction->setText("forward");
    mp_historyForwardAction->setToolTip("forward");
    connect(mp_historyForwardAction, &QAction::triggered, [this](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });
    addAction(mp_historyForwardAction);
    addSeparator();

    addWidget(&m_searchEntry);

    addSeparator();

#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/minimize.svg"), "minimize", parent, SLOT(showMinimized()));
#endif
    mp_fullScreenAction = addAction(QIcon(":/icons/full-screen-enter.svg"), "fullscreen", this, SLOT(toggleFullScreen()));
    mp_normalScreenAction = addAction(QIcon(":/icons/full-screen-exit.svg"), "unfullscreen", this, SLOT(toggleFullScreen()));
    mp_normalScreenAction->setVisible(false);
#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/close.svg"), "close", parent, SLOT(close()));
#endif
    setMovable(false);
}

TopWidget::~TopWidget()
{
    delete mp_historyBackAction;
    delete mp_historyForwardAction;
    delete mp_fullScreenAction;
    delete mp_normalScreenAction;
}


void TopWidget::toggleFullScreen() {
    if (m_fullScreen)
        parentWidget()->showNormal();
    else
        parentWidget()->showFullScreen();
    m_fullScreen = !m_fullScreen;
    mp_fullScreenAction->setVisible(!m_fullScreen);
    mp_normalScreenAction->setVisible(m_fullScreen);
}

void TopWidget::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    switch (action) {
    case QWebEnginePage::Back:
        mp_historyBackAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        mp_historyForwardAction->setEnabled(enabled);
        break;
    default:
        break;
    }
}


void TopWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() != Qt::LeftButton)
        return;

    m_cursorPos = event->globalPos() + frameGeometry().topLeft() - parentWidget()->frameGeometry().topLeft();
    m_timestamp = event->timestamp();
    event->accept();
}

void TopWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->timestamp() <= m_timestamp)
        return;

    m_timestamp = event->timestamp();
    auto delta = event->globalPos() - m_cursorPos;
    parentWidget()->move(delta);
    event->accept();
}
