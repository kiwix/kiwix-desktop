#include "kiwixapp.h"
#include "topwidget.h"
#include "kconstants.h"
#include "mainmenu.h"

#include <QMouseEvent>
#include <QAction>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent)
{
    mp_historyBackAction = new QAction(this);
    mp_historyBackAction->setIcon(QIcon(":/icons/back.svg"));
    mp_historyBackAction->setText(gt("back"));
    mp_historyBackAction->setToolTip(gt("back"));
    mp_historyBackAction->setEnabled(false);
    connect(mp_historyBackAction, &QAction::triggered, [](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });
    addAction(mp_historyBackAction);
    widgetForAction(mp_historyBackAction)->setObjectName("backButton");
    mp_historyForwardAction = new QAction(this);
    mp_historyForwardAction->setIcon(QIcon(":/icons/forward.svg"));
    mp_historyForwardAction->setText(gt("forward"));
    mp_historyForwardAction->setToolTip(gt("forward"));
    mp_historyForwardAction->setEnabled(false);
    connect(mp_historyForwardAction, &QAction::triggered, [](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });
    addAction(mp_historyForwardAction);
    addSeparator();

    addWidget(&m_searchEntry);
    addAction(KiwixApp::instance()->getAction(KiwixApp::ToggleReadingListAction));

    addSeparator();

    addAction(KiwixApp::instance()->getAction(KiwixApp::OpenFileAction));

    QMenu* menu = new MainMenu();
    QAction* menuAction = new QAction(this);
    menuAction->setIcon(QIcon(":/icons/more.svg"));
    menuAction->setMenu(menu);
    menuAction->setToolTip(gt("main-menu"));

    addAction(menuAction);
    setContextMenuPolicy( Qt::PreventContextMenu );


#if !SYSTEMTITLEBAR
    addAction(QIcon(":/icons/minimize.svg"), "minimize", parent, SLOT(showMinimized()));
#endif
    addAction(KiwixApp::instance()->getAction(KiwixApp::ToggleFullscreenAction));
    widgetForAction(KiwixApp::instance()->getAction(KiwixApp::ToggleFullscreenAction))->setObjectName("fullScreenButton");
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
