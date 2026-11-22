#include "topwidget.h"

#include "kconstants.h"
#include "kiwixapp.h"
#include "mainmenu.h"
#include "tabbar.h"
#include "portutils.h"

#include <QMouseEvent>
#include <QAction>
#include <QToolButton>

TopWidget::TopWidget(QWidget *parent) :
    QToolBar(parent)
{
    auto app = KiwixApp::instance();

    QAction *back = app->getAction(KiwixApp::HistoryBackAction);
    connect(back, &QAction::triggered, [app](){
        app->getTabWidget()->triggerWebPageAction(QWebEnginePage::Back);
    });
    addAction(back);

    QAction *forward = app->getAction(KiwixApp::HistoryForwardAction);
    connect(forward, &QAction::triggered, [app](){
        app->getTabWidget()->triggerWebPageAction(QWebEnginePage::Forward);
    });
    addAction(forward);

    QAction *random = app->getAction(KiwixApp::RandomArticleAction);
    addAction(random);

    QAction *toc = app->getAction(KiwixApp::ToggleTOCAction);
    addAction(toc);

    // For CSS
    if (QGuiApplication::isLeftToRight()) {
        widgetForAction(back)->setObjectName("leftHistoryButton");
        widgetForAction(forward)->setObjectName("rightHistoryButton");
    } else {
        widgetForAction(forward)->setObjectName("leftHistoryButton");
        widgetForAction(back)->setObjectName("rightHistoryButton");
    }

    addSeparator();

    addAction(app->getAction(KiwixApp::OpenHomePageAction));

    addWidget(&m_searchEntry);

    addAction(app->getAction(KiwixApp::ToggleReadingListAction));

    addSeparator();

    addAction(app->getAction(KiwixApp::OpenFileAction));

    QMenu* menu = new MainMenu();
    QToolButton *toolButton = new QToolButton(menu);
    toolButton->setIcon(themeAwareIcon("more"));
    toolButton->setPopupMode(QToolButton::InstantPopup);
    toolButton->setToolTip(gt("main-menu"));
    toolButton->setMenu(menu);
    addWidget(toolButton);

    setContextMenuPolicy( Qt::PreventContextMenu );

#if !SYSTEMTITLEBAR
    QAction *minimizeAction = addAction(themeAwareIcon("minimize"), "minimize", parent, SLOT(showMinimized()));
#endif
    addAction(app->getAction(KiwixApp::ToggleFullscreenAction));
    widgetForAction(app->getAction(KiwixApp::ToggleFullscreenAction))->setObjectName("fullScreenButton");
#if !SYSTEMTITLEBAR
    QAction *closeAction = addAction(themeAwareIcon("close"), "close", parent, SLOT(close()));
#endif
    connect(app->getSettingsManager(), &SettingsManager::themeChanged,
            this, [=](SettingsManager::Theme) {
        toolButton->setIcon(themeAwareIcon("more"));
#if !SYSTEMTITLEBAR
        minimizeAction->setIcon(themeAwareIcon("minimize"));
        closeAction->setIcon(themeAwareIcon("close"));
#endif
    });
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

    QPoint globalPos = portutils::getGlobalPos(*event);
    m_cursorPos = globalPos + frameGeometry().topLeft() - parentWidget()->frameGeometry().topLeft();
    m_timestamp = event->timestamp();
    event->accept();
}

void TopWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->timestamp() <= m_timestamp)
        return;

    QPoint globalPos = portutils::getGlobalPos(*event);
    m_timestamp = event->timestamp();
    auto delta = globalPos - m_cursorPos;
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
