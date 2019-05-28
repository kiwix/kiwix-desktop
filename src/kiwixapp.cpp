#include "kiwixapp.h"
#include "zim/error.h"

#include <QLocale>
#include <QLibraryInfo>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QFile>
#include <QFileDialog>
#include <QAction>
#include <QPrinter>
#include <QPrintDialog>
#include <thread>

kiwix::Downloader* createDownloader() {
    int attempt = 5;
    while(attempt--) {
        try {
            return new kiwix::Downloader();
        } catch (exception& e) {
            qInfo() << "Cannot create downloader" << e.what();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    return nullptr;
}

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv),
      m_library(),
      mp_downloader(createDownloader()),
      m_manager(&m_library, mp_downloader)
{
    m_qtTranslator.load(QLocale(), "qt", "_",
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslator(&m_qtTranslator);

    m_appTranslator.load(QLocale(), "kiwix-desktop", "_", ":/i18n/");
    installTranslator(&m_appTranslator);

    auto icon = QIcon();
    icon.addFile(":/icons/kiwix-app-icons-square.svg");
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
    mp_tabWidget = mp_mainWindow->getTabBar();
    mp_tabWidget->setContentManagerView(m_manager.getView());
    mp_mainWindow->getSideContentManager()->setContentManager(&m_manager);
    setSideBar(CONTENTMANAGER_BAR);
    postInit();

    mp_errorDialog = new QErrorMessage(mp_mainWindow);
    mp_mainWindow->show();
    if (!mp_downloader) {
        showMessage("Impossible to launch downloader");
        mpa_actions[ExitAction]->trigger();
    }
}

KiwixApp::~KiwixApp()
{
    if (mp_downloader) {
        mp_downloader->close();
        delete mp_downloader;
    }
    delete mp_errorDialog;
    delete mp_mainWindow;
}

KiwixApp *KiwixApp::instance()
{
    return static_cast<KiwixApp*>(QApplication::instance());
}

void KiwixApp::openZimFile(const QString &zimfile)
{
    QString _zimfile = zimfile;
    if (_zimfile.isEmpty()) {
        _zimfile = QFileDialog::getOpenFileName(
            getMainWindow(),
            tr("Open Zim"),
            QString(),
            "ZimFile (*.zim*)");
        _zimfile = QDir::toNativeSeparators(_zimfile);
    }
    if (_zimfile.isEmpty()) {
        return;
    }
    QString zimId;
    try {
        zimId = m_library.openBookFromPath(_zimfile);
    } catch (const std::exception& e) {
        showMessage("Cannot open " + _zimfile + ": \n" + e.what());
        return;
    }
    openUrl(QUrl("zim://"+zimId+".zim/"));
}

void KiwixApp::printPage()
{
    if(!mp_tabWidget->currentWidget())
        return;
    QPrinter* printer = new QPrinter();
    QPrintDialog printDialog(printer, mp_mainWindow);
    printDialog.setStyle(nullptr);
    printDialog.setStyleSheet("");
    if (printDialog.exec() == QDialog::Accepted) {
        auto webview = mp_tabWidget->currentWidget();
        if(!webview)
            return;
        webview->page()->print(printer, [=](bool success) {
            if (!success) {
                showMessage("An error has occured while printing.");
            }
            delete printer;
        });
    }
}

void KiwixApp::openUrl(const QString &url, bool newTab) {
    openUrl(QUrl(url), newTab);
}

void KiwixApp::openUrl(const QUrl &url, bool newTab) {
    mp_tabWidget->openUrl(url, newTab);
}

void KiwixApp::setSideBar(KiwixApp::SideBarType type)
{
    auto sideDockWidget = mp_mainWindow->getSideDockWidget();
    switch(type) {
        case SEARCH_BAR:
        case CONTENTMANAGER_BAR:
        case READINGLIST_BAR:
            sideDockWidget->setCurrentIndex(type);
            sideDockWidget->show();
            break;
        case NONE:
            sideDockWidget->hide();
            break;
    }
    m_currentSideType = type;
    emit(currentSideTypeChanged(type));
}

void KiwixApp::toggleSideBar(KiwixApp::SideBarType type) {
    if (m_currentSideType == type) {
        setSideBar(NONE);
        return;
    }

    if (m_currentSideType == CONTENTMANAGER_BAR) {
        return;
    }
    setSideBar(type);
}

void KiwixApp::openRandomUrl(bool newTab)
{
    auto zimId = mp_tabWidget->currentZimId();
    if (zimId.isEmpty()) {
        return;
    }
    auto reader = m_library.getReader(zimId);
    auto entry = reader->getRandomPage();

    QUrl url;
    url.setScheme("zim");
    url.setHost(zimId + ".zim");
    url.setPath("/" + QString::fromStdString(entry.getPath()));
    openUrl(url, newTab);
}

void KiwixApp::showMessage(const QString &message)
{
    mp_errorDialog->showMessage(message);
}

QAction *KiwixApp::getAction(KiwixApp::Actions action)
{
    return mpa_actions[action];
}

bool KiwixApp::isCurrentArticleBookmarked()
{
    auto zimId = getTabWidget()->currentZimId().toStdString();
    if (zimId.empty()) {
        return false;
    }
    auto url = getTabWidget()->currentArticleUrl().toStdString();

    for (auto& bookmark: getLibrary()->getBookmarks()) {
        if (bookmark.getBookId() == zimId && bookmark.getUrl() == url) {
            return true;
        }
    }
    return false;
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
    CREATE_ACTION_ICON(KiwixServeAction, "share", tr("Local Kiwix Server"));
    SET_SHORTCUT(KiwixServeAction, QKeySequence(Qt::CTRL+Qt::Key_I));
    HIDE_ACTION(KiwixServeAction);

    CREATE_ACTION_ICON(RandomArticleAction, "random", tr("Random Article"));
    SET_SHORTCUT(RandomArticleAction, QKeySequence(Qt::CTRL+Qt::Key_R));
    connect(mpa_actions[RandomArticleAction], &QAction::triggered,
            this, [=]() { this->openRandomUrl(false); });

    CREATE_ACTION_ICON(PrintAction, "print", tr("Print"));
    SET_SHORTCUT(PrintAction, QKeySequence::Print);
    connect(mpa_actions[PrintAction], &QAction::triggered,
            this, &KiwixApp::printPage);

    CREATE_ACTION(NewTabAction, tr("New tab"));
    SET_SHORTCUT(NewTabAction, QKeySequence::AddTab);

    CREATE_ACTION(CloseTabAction, tr("Close tab"));
    SET_SHORTCUT(CloseTabAction, QKeySequence::Close);

    CREATE_ACTION(ReopenClosedTabAction, tr("Reopen closed tab"));
    SET_SHORTCUT(ReopenClosedTabAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_T));
    HIDE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION(BrowseLibraryAction, tr("Browse library"));
    SET_SHORTCUT(BrowseLibraryAction, QKeySequence(Qt::CTRL+Qt::Key_E));
    HIDE_ACTION(BrowseLibraryAction);

    CREATE_ACTION(OpenFileAction, tr("Open file"));
    SET_SHORTCUT(OpenFileAction, QKeySequence::Open);
    connect(mpa_actions[OpenFileAction], &QAction::triggered,
            this, [=]() { openZimFile(); });

    CREATE_ACTION(OpenRecentAction, tr("Open recent"));
    HIDE_ACTION(OpenRecentAction);

    CREATE_ACTION(SavePageAsAction, tr("Save page as ..."));
    SET_SHORTCUT(SavePageAsAction, QKeySequence::SaveAs);
    HIDE_ACTION(SavePageAsAction);

    CREATE_ACTION(SearchArticleAction, tr("Search article"));
    SET_SHORTCUT(SearchArticleAction, QKeySequence(Qt::CTRL+Qt::Key_L));
    HIDE_ACTION(SearchArticleAction);

    CREATE_ACTION(SearchLibraryAction, tr("Search in library"));
    SET_SHORTCUT(SearchLibraryAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, tr("Find in page"));
    SET_SHORTCUT(FindInPageAction, QKeySequence::Find);
    connect(mpa_actions[FindInPageAction], &QAction::triggered,
            this, [=]() { toggleSideBar(SEARCH_BAR); });

    CREATE_ACTION_ICON(ToggleFullscreenAction, "full-screen-enter", tr("Set fullScreen"));
    SET_SHORTCUT(ToggleFullscreenAction, QKeySequence::FullScreen);
    connect(mpa_actions[ToggleFullscreenAction], &QAction::toggled,
            this, [=](bool checked) {
        auto action = mpa_actions[ToggleFullscreenAction];
        action->setIcon(
            QIcon(checked ? ":/icons/full-screen-exit.svg" : ":/icons/full-screen-enter.svg"));
        action->setText(checked ? tr("Quit fullScreen") : tr("Set fullScreen"));
    });
    mpa_actions[ToggleFullscreenAction]->setCheckable(true);

    CREATE_ACTION(ToggleTOCAction, tr("Table of content"));
    SET_SHORTCUT(ToggleTOCAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_1));
    HIDE_ACTION(ToggleTOCAction);

    CREATE_ACTION_ICON(ToggleReadingListAction, "reading-list" ,tr("Reading list"));
    SET_SHORTCUT(ToggleReadingListAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_2));
    connect(mpa_actions[ToggleReadingListAction], &QAction::triggered,
            this, [=]() { toggleSideBar(READINGLIST_BAR); });
    connect(this, &KiwixApp::currentSideTypeChanged,
            this, [=](SideBarType type) {
        mpa_actions[ToggleReadingListAction]->setIcon(
            QIcon((type == READINGLIST_BAR) ? ":/icons/reading-list-active.svg" : ":/icons/reading-list.svg"));
    });

    CREATE_ACTION(ZoomInAction, tr("Zoom in"));
    SET_SHORTCUT(ZoomInAction, QKeySequence::ZoomIn);

    CREATE_ACTION(ZoomOutAction, tr("Zoom out"));
    SET_SHORTCUT(ZoomOutAction, QKeySequence::ZoomOut);

    CREATE_ACTION(ZoomResetAction, tr("Zoom reset"));
    SET_SHORTCUT(ZoomResetAction, QKeySequence(Qt::CTRL+Qt::Key_0));

    CREATE_ACTION(HelpAction, tr("Help"));
    SET_SHORTCUT(HelpAction, QKeySequence::HelpContents);
    HIDE_ACTION(HelpAction);

    CREATE_ACTION(FeedbackAction, tr("Feedback"));
    HIDE_ACTION(FeedbackAction);

    CREATE_ACTION(ReportBugAction, tr("Repost a bug"));
    HIDE_ACTION(ReportBugAction);

    CREATE_ACTION(RequestFeatureAction, tr("Request a feature"));
    HIDE_ACTION(RequestFeatureAction);

    CREATE_ACTION(AboutAction, tr("About Kiwix"));

    CREATE_ACTION_ICON(SettingAction, "settings", tr("Settings"));
    SET_SHORTCUT(SettingAction, QKeySequence::Preferences);
    HIDE_ACTION(SettingAction);

    CREATE_ACTION_ICON(DonateAction, "donate", tr("Donate to support Kiwix"));
    //SET_SHORTCUT(DonateAction, QKeySequence(Qt::CTRL+Qt::Key_BracketLeft+Qt::Key_3));
    HIDE_ACTION(DonateAction);

    CREATE_ACTION_ICON(ExitAction, "exit", tr("Exit"));
    SET_SHORTCUT(ExitAction, QKeySequence::Quit);
}

void KiwixApp::postInit() {
    connect(mp_tabWidget, &TabBar::webActionEnabledChanged,
            mp_mainWindow->getTopWidget(), &TopWidget::handleWebActionEnabledChanged);
    connect(mp_tabWidget, &TabBar::currentTitleChanged, this,
            [=](const QString& title) { emit currentTitleChanged(title); });
    emit(m_library.booksChanged());
}
