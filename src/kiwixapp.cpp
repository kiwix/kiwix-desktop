#include "kiwixapp.h"
#include "zim/error.h"

#include <QFontDatabase>
#include <QStyleFactory>
#include <QFile>
#include <QAction>

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv)
{
    auto icon = QIcon();
    icon.addFile(":/icons/kiwix/app_icon.svg");
    setWindowIcon(icon);

    setApplicationDisplayName("Kiwix");
    setApplicationName("Kiwix");
    setDesktopFileName("kiwix.desktop");

    setStyle(QStyleFactory::create("Windows"));
    QFile styleFile(":/css/style.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    setStyleSheet(style);


    QString fontName;
    if (platformName() == "windows") {
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuib.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuii.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuil.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuisl.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeui.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuiz.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguibli.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguibl.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguili.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguisbi.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguisb.ttf");
        fontName = "Segoe";
    } else {
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Regular.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Light.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-LightItalic.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Medium.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-MediumItalic.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Bold.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-BoldItalic.ttf");
        fontName = "Ubuntu";
    }

    auto font = QFont(fontName);
    setFont(font);

    createAction();
    mp_mainWindow = new MainWindow;
    mp_mainWindow->show();
    mp_tabWidget = mp_mainWindow->getTabWidget();

    mp_errorDialog = new QErrorMessage(mp_mainWindow);
}

KiwixApp::~KiwixApp()
{
    delete mp_errorDialog;
    delete mp_mainWindow;
}

KiwixApp *KiwixApp::instance()
{
    return static_cast<KiwixApp*>(QApplication::instance());
}

void KiwixApp::openZimFile(const QString &zimfile)
{
    QString zimId;
    try {
        zimId = m_library.openBook(zimfile);
    } catch (const std::exception& e) {
        showMessage("Cannot open " + zimfile + ": \n" + e.what());
        return;
    }
    openUrl(QUrl("zim://"+zimId+"/"));
}

void KiwixApp::openUrl(const QUrl &url, bool newTab) {
    mp_tabWidget->openUrl(url, newTab);
}

void KiwixApp::showMessage(const QString &message)
{
    mp_errorDialog->showMessage(message);
}

QAction *KiwixApp::getAction(KiwixApp::Actions action)
{
    return mpa_actions[action];
}

#define CREATE_ACTION_ICON(ID, ICON, TEXT) \
    mpa_actions[ID] = new QAction(QIcon(":/icons/" ICON ".svg"), TEXT)
#define CREATE_ACTION(ID, TEXT) \
    mpa_actions[ID] = new QAction(TEXT)
#define SET_SHORTCUT(ID, SHORTCUT) mpa_actions[ID]->setShortcut(SHORTCUT)
#define HIDE_ACTION(ID) mpa_actions[ID]->setVisible(false)
#define DISABLE_ACTION(ID) mpa_actions[ID]->setDisabled(true)

void KiwixApp::createAction()
{
    CREATE_ACTION_ICON(KiwixServeAction, "share", "Local Kiwix Server");
    SET_SHORTCUT(KiwixServeAction, QKeySequence(Qt::CTRL+Qt::Key_I));
    HIDE_ACTION(KiwixServeAction);

    CREATE_ACTION_ICON(RandomArticleAction, "random", "Random Article");
    SET_SHORTCUT(RandomArticleAction, QKeySequence(Qt::CTRL+Qt::Key_R));
    DISABLE_ACTION(RandomArticleAction);

    CREATE_ACTION_ICON(PrintAction, "print", "Print");
    SET_SHORTCUT(PrintAction, QKeySequence::Print);
    DISABLE_ACTION(PrintAction);

    CREATE_ACTION(NewTabAction, "New tab");
    SET_SHORTCUT(NewTabAction, QKeySequence::AddTab);

    CREATE_ACTION(CloseTabAction, "Close tab");
    SET_SHORTCUT(CloseTabAction, QKeySequence::Close);

    CREATE_ACTION(ReopenClosedTabAction, "Reopen closed tab");
    SET_SHORTCUT(ReopenClosedTabAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_T));
    DISABLE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION(BrowseLibraryAction, "Browse library");
    SET_SHORTCUT(BrowseLibraryAction, QKeySequence(Qt::CTRL+Qt::Key_E));
    HIDE_ACTION(BrowseLibraryAction);

    CREATE_ACTION(OpenFileAction, "Open file");
    SET_SHORTCUT(OpenFileAction, QKeySequence::Open);
    DISABLE_ACTION(OpenFileAction);

    CREATE_ACTION(OpenRecentAction, "Open recent");
    HIDE_ACTION(OpenRecentAction);

    CREATE_ACTION(SavePageAsAction, "Save page as ...");
    SET_SHORTCUT(SavePageAsAction, QKeySequence::SaveAs);
    HIDE_ACTION(SavePageAsAction);

    CREATE_ACTION(SearchArticleAction, "Search article");
    SET_SHORTCUT(SearchArticleAction, QKeySequence(Qt::CTRL+Qt::Key_L));
    HIDE_ACTION(SearchArticleAction);

    CREATE_ACTION(SearchLibraryAction, "Search in library");
    SET_SHORTCUT(SearchLibraryAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, "Find in page");
    SET_SHORTCUT(FindInPageAction, QKeySequence::Find);
    HIDE_ACTION(FindInPageAction);

    CREATE_ACTION(FindNextAction, "Find next");
    SET_SHORTCUT(FindNextAction, QKeySequence::FindNext);
    HIDE_ACTION(FindNextAction);

    CREATE_ACTION(FindPreviousAction, "Find previous");
    SET_SHORTCUT(FindPreviousAction, QKeySequence::FindPrevious);
    HIDE_ACTION(FindPreviousAction);

    CREATE_ACTION(ToggleFullscreenAction, "Set full screen");
    SET_SHORTCUT(ToggleFullscreenAction, QKeySequence::FullScreen);
    DISABLE_ACTION(ToggleFullscreenAction);

    CREATE_ACTION(ToggleTOCAction, "Table of content");
    SET_SHORTCUT(ToggleTOCAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_1));
    HIDE_ACTION(ToggleTOCAction);

    CREATE_ACTION(ToggleReadingListAction, "Reading list");
    SET_SHORTCUT(ToggleReadingListAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_2));
    HIDE_ACTION(ToggleReadingListAction);

    CREATE_ACTION(ZoomInAction, "Zoom in");
    SET_SHORTCUT(ZoomInAction, QKeySequence::ZoomIn);
    HIDE_ACTION(ZoomInAction);

    CREATE_ACTION(ZoomOutAction, "Zoom out");
    SET_SHORTCUT(ZoomOutAction, QKeySequence::ZoomOut);
    HIDE_ACTION(ZoomOutAction);

    CREATE_ACTION(ZoomResetAction, "Zoom reset");
    SET_SHORTCUT(ZoomResetAction, QKeySequence(Qt::CTRL+Qt::Key_0));
    HIDE_ACTION(ZoomResetAction);

    CREATE_ACTION(HelpAction, "Help");
    SET_SHORTCUT(HelpAction, QKeySequence::HelpContents);
    HIDE_ACTION(HelpAction);

    CREATE_ACTION(FeedbackAction, "Feedback");
    HIDE_ACTION(FeedbackAction);

    CREATE_ACTION(ReportBugAction, "Repost a bug");
    HIDE_ACTION(ReportBugAction);

    CREATE_ACTION(RequestFeatureAction, "Request a feature");
    HIDE_ACTION(RequestFeatureAction);

    CREATE_ACTION(AboutAction, "About Kiwix");
    HIDE_ACTION(AboutAction);

    CREATE_ACTION_ICON(SettingAction, "settings", "Settings");
    SET_SHORTCUT(SettingAction, QKeySequence::Preferences);
    DISABLE_ACTION(SettingAction);

    CREATE_ACTION_ICON(DonateAction, "donate", "Donate to support Kiwix");
    //SET_SHORTCUT(DonateAction, QKeySequence(Qt::CTRL+Qt::Key_BracketLeft+Qt::Key_3));
    DISABLE_ACTION(DonateAction);

    CREATE_ACTION_ICON(ExitAction, "exit", "Exit");
    SET_SHORTCUT(ExitAction, QKeySequence::Quit);
}
