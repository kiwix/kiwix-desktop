#include "mainmenu.h"
#include "kiwixapp.h"
#include "menuproxystyle.h"

#include <QTranslator>

#define ADD_ACTION(ID) addAction(app->getAction(KiwixApp::ID));

MainMenu::MainMenu(QWidget *parent) :
    QMenu(parent)
{
    auto app = KiwixApp::instance();

    auto menuStyle = new MenuProxyStyle();
    // Qt's Windows 11 style adds these flags to menus while polishing them.
    // Changing the flags of a menu with its own style (MenuProxyStyle) under
    // an application style sheet replaces and frees the style sheet style
    // that is running the polish, which crashes at startup. With the flags
    // already set, the style's setWindowFlag() calls are no-ops.
    if (menuStyle->baseStyle()->name() == QLatin1String("windows11"))
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setStyle(menuStyle);

    ADD_ACTION(KiwixServeAction);
    ADD_ACTION(RandomArticleAction);
    ADD_ACTION(PrintAction);
    addSeparator();

    m_fileMenu.setTitle(gt("file"));
    m_fileMenu.ADD_ACTION(NewTabAction);
    m_fileMenu.ADD_ACTION(CloseCurrentTabAction);
    m_fileMenu.ADD_ACTION(ReopenClosedTabAction);
    m_fileMenu.ADD_ACTION(BrowseLibraryAction);
    m_fileMenu.ADD_ACTION(OpenFileAction);
    m_fileMenu.ADD_ACTION(OpenRecentAction);
    m_fileMenu.ADD_ACTION(ExportReadingListAction);
    m_fileMenu.ADD_ACTION(ImportReadingListAction);
    m_fileMenu.ADD_ACTION(SavePageAsAction);

    addMenu(&m_fileMenu);

    m_editMenu.setTitle(gt("edit"));
    m_editMenu.ADD_ACTION(SearchArticleAction);
    m_editMenu.ADD_ACTION(SearchLibraryAction);
    m_editMenu.ADD_ACTION(FindInPageAction);
    m_editMenu.ADD_ACTION(ToggleAddBookmarkAction);
    m_editMenu.ADD_ACTION(OpenMultiZimAction);
#if defined(QT_TEXTTOSPEECH_LIB)
    m_editMenu.ADD_ACTION(ReadArticleAction);
    m_editMenu.ADD_ACTION(ReadTextAction);
    m_editMenu.ADD_ACTION(ReadStopAction);
#endif
    addMenu(&m_editMenu);

    m_viewMenu.setTitle(gt("view"));
    m_viewMenu.ADD_ACTION(ToggleFullscreenAction);
    m_viewMenu.ADD_ACTION(ToggleTOCAction);
    m_viewMenu.ADD_ACTION(ToggleReadingListAction);
    m_viewMenu.ADD_ACTION(ToggleTTSLanguageAction);
    m_viewMenu.ADD_ACTION(ToggleTTSVoiceAction);
    m_viewMenu.ADD_ACTION(IncreaseTTSSpeedAction);
    m_viewMenu.ADD_ACTION(DecreaseTTSSpeedAction);
    m_viewMenu.ADD_ACTION(ZoomInAction);
    m_viewMenu.ADD_ACTION(ZoomOutAction);
    m_viewMenu.ADD_ACTION(ZoomResetAction);
    addMenu(&m_viewMenu);

    m_toolsMenu.setTitle(gt("tools"));
//    m_toolsMenu.addAction();
//    addMenu(&m_toolsMenu);

    m_helpMenu.setTitle(gt("help"));
    m_helpMenu.ADD_ACTION(HelpAction);
    m_helpMenu.ADD_ACTION(FeedbackAction);
    m_helpMenu.ADD_ACTION(ReportBugAction);
    m_helpMenu.ADD_ACTION(RequestFeatureAction);
    m_helpMenu.ADD_ACTION(AboutAction);
    addMenu(&m_helpMenu);

    addSeparator();
    ADD_ACTION(SettingAction);
    ADD_ACTION(DonateAction);
    ADD_ACTION(ExitAction);
}
