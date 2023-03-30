#include "topwidget.h"

#include "kconstants.h"
#include "kiwixapp.h"
#include "mainmenu.h"
#include "tabbar.h"

#include <QMouseEvent>
#include <QAction>
#include <QToolButton>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent)
{
    auto app = KiwixApp::instance();

    QAction *back = app->getAction(KiwixApp::HistoryBackAction);
    connect(back, &QAction::triggered, [](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });
    addAction(back);

    QAction *forward = app->getAction(KiwixApp::HistoryForwardAction);
    connect(forward, &QAction::triggered, [](){
        KiwixApp::instance()->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });
    addAction(forward);

    QAction *random = app->getAction(KiwixApp::RandomArticleAction);
    addAction(random);

    // For CSS
    if (QGuiApplication::isLeftToRight()) {
        widgetForAction(back)->setObjectName("leftHistoryButton");
        widgetForAction(forward)->setObjectName("rightHistoryButton");
    } else {
        widgetForAction(forward)->setObjectName("leftHistoryButton");
        widgetForAction(back)->setObjectName("rightHistoryButton");
    }

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
}

void TopWidget::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    auto app = KiwixApp::instance();

    switch (action) {
    case QWebEnginePage::Back:
        app->getAction(KiwixApp::HistoryBackAction)->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        app->getAction(KiwixApp::HistoryForwardAction)->setEnabled(enabled);
        break;
    default:
        break;
    }
}


void TopWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton)
        return;

    m_cursorPos = event->globalPos() + frameGeometry().topLeft() - parentWidget()->frameGeometry().topLeft();
    m_timestamp = event->timestamp();
    event->accept();
}

void TopWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->timestamp() <= m_timestamp)
        return;

    m_timestamp = event->timestamp();
    auto delta = event->globalPos() - m_cursorPos;
    parentWidget()->move(delta);
    event->accept();
}

QToolButton* TopWidget::getBackButton() const
{
    auto app = KiwixApp::instance();
    QAction *back = app->getAction(KiwixApp::HistoryBackAction);
    return qobject_cast<QToolButton*>(widgetForAction(back));
}

QToolButton* TopWidget::getForwardButton() const
{
    auto app = KiwixApp::instance();
    QAction *forward = app->getAction(KiwixApp::HistoryForwardAction);
    return qobject_cast<QToolButton*>(widgetForAction(forward));
}

void TopWidget::updateBackForwardButtons()
{
    WebView *webview = KiwixApp::instance()->getTabWidget()->currentWebView();

    if (webview) {
        back_menu.reset(webview->getHistoryBackMenu());
        forward_menu.reset(webview->getHistoryForwardMenu());
    } else {
        back_menu.reset();
        forward_menu.reset();
    }

    getBackButton()->setMenu(back_menu.data());
    getForwardButton()->setMenu(forward_menu.data());
}
