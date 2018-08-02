#include "topwidget.h"

#include "kconstants.h"
#include "kiwixapp.h"
#include "mainmenu.h"

#include <QMouseEvent>
#include <QAction>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent)
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


    QMenu* menu = new MainMenu();
    QAction* menuAction = new QAction(this);
    menuAction->setIcon(QIcon(":/icons/more.svg"));
    menuAction->setMenu(menu);

    addAction(menuAction);


#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/minimize.svg"), "minimize", parent, SLOT(showMinimized()));
#endif
    addAction(KiwixApp::instance()->getAction(KiwixApp::ToggleFullscreenAction));
#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/close.svg"), "close", parent, SLOT(close()));
#endif
    setMovable(false);
}

TopWidget::~TopWidget()
{
    delete mp_historyBackAction;
    delete mp_historyForwardAction;
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
