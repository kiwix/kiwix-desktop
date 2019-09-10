#include <QTranslator>

#include "mainmenu.h"
#include "kiwixapp.h"

#define ADD_ACTION(ID) addAction(app->getAction(KiwixApp::ID));

MainMenu::MainMenu(QWidget *parent) :
    QMenu(parent)
{
    auto app = KiwixApp::instance();

    ADD_ACTION(KiwixServeAction);
    ADD_ACTION(RandomArticleAction);
    ADD_ACTION(PrintAction);
    addSeparator();

    m_fileMenu.setTitle(tr("File"));
    m_fileMenu.ADD_ACTION(NewTabAction);
    m_fileMenu.ADD_ACTION(CloseTabAction);
    m_fileMenu.ADD_ACTION(ReopenClosedTabAction);
    m_fileMenu.ADD_ACTION(BrowseLibraryAction);
    m_fileMenu.ADD_ACTION(OpenFileAction);
    m_fileMenu.ADD_ACTION(OpenRecentAction);

    /* TODO See https://github.com/kiwix/kiwix-desktop/issues/77
    m_fileMenu.ADD_ACTION(SavePageAsAction);
    */
    
    addMenu(&m_fileMenu);

    m_editMenu.setTitle(tr("Edit"));
    m_editMenu.ADD_ACTION(SearchArticleAction);
    m_editMenu.ADD_ACTION(SearchLibraryAction);
    m_editMenu.ADD_ACTION(FindInPageAction);
    addMenu(&m_editMenu);

    m_viewMenu.setTitle(tr("View"));
    m_viewMenu.ADD_ACTION(ToggleFullscreenAction);
    m_viewMenu.ADD_ACTION(ToggleTOCAction);
    m_viewMenu.ADD_ACTION(ToggleReadingListAction);
    m_viewMenu.ADD_ACTION(ZoomInAction);
    m_viewMenu.ADD_ACTION(ZoomOutAction);
    m_viewMenu.ADD_ACTION(ZoomResetAction);
    addMenu(&m_viewMenu);

    m_toolsMenu.setTitle(tr("Tools"));
//    m_toolsMenu.addAction();
//    addMenu(&m_toolsMenu);

    m_helpMenu.setTitle(tr("Help"));
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
