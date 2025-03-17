#include "kiwixapp.h"
#include "zim/error.h"
#include "zim/version.h"
#include "kiwix/tools.h"
#include "kiwix/version.h"

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
#include <QMessageBox>
#if defined(Q_OS_WIN)
#include <QWindow>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#else
#include <qpa/qplatformwindow_p.h>
#endif
#endif

const QString DEFAULT_SAVE_DIR = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

////////////////////////////////////////////////////////////////////////////////
// KiwixApp
////////////////////////////////////////////////////////////////////////////////

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QtSingleApplication("kiwix-desktop", argc, argv),
      m_profile(),
      m_libraryDirectory(findLibraryDirectory()),
      m_library(m_libraryDirectory),
      mp_manager(nullptr),
      mp_mainWindow(nullptr),
      mp_nameMapper(std::make_shared<kiwix::UpdatableNameMapper>(m_library.getKiwixLibrary(), false)),
      m_server(m_library.getKiwixLibrary(), mp_nameMapper),
      mp_session(nullptr)
{
    /* Place session file in our global library path */
    QDir dir(m_libraryDirectory);
    mp_session = new QSettings(dir.filePath("kiwix-desktop.session"),
                               QSettings::IniFormat, this);
    try {
        m_translation.setTranslation(QLocale());
    } catch (std::exception& e) {
        QMessageBox::critical(nullptr, "Translation error", e.what());
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QString path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
    QString path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif
    loadAndInstallTranslations(m_qtTranslator, "qt", path);
    loadAndInstallTranslations(m_appTranslator, "kiwix-desktop", ":/i18n/");

    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawkb.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawkl.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawksb.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawksl.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawk.ttf");
    setFont(QFont("Selawik"));
}

void KiwixApp::loadAndInstallTranslations(QTranslator& translator, const QString& filename, const QString& directory) {
    if (translator.load(QLocale(), filename, "_", directory)) {
        installTranslator(&translator);
    }
}

void KiwixApp::init()
{
    mp_manager = new ContentManager(&m_library);
    mp_manager->setLocal(!m_library.getBookIds().isEmpty());

    auto icon = QIcon();
    icon.addFile(":/icons/kiwix-app-icons-square.svg");
    setWindowIcon(icon);

    setApplicationName("Kiwix");
    setDesktopFileName("kiwix.desktop");
    setStyleSheet(parseStyleFromFile(":/css/style.css"));

    createActions();
    mp_mainWindow = new MainWindow;
    getTabWidget()->setContentManagerView(mp_manager->getView());
    const auto newTabAction = getAction(KiwixApp::NewTabAction);
    getTabWidget()->setNewTabButton(newTabAction);
    connect(newTabAction, &QAction::triggered, this, &KiwixApp::newTab);
    postInit();
    mp_errorDialog = new QErrorMessage(mp_mainWindow);
    setActivationWindow(mp_mainWindow);
    mp_mainWindow->show();
#ifdef Q_OS_WIN
    QWindow *window = mp_mainWindow->windowHandle();
    if (window) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QWindowsWindowFunctions::setHasBorderInFullScreen(window, true);
#else
        auto nativeWindow = window->nativeInterface<QNativeInterface::Private::QWindowsWindow>();
        if (nativeWindow) {
            nativeWindow->setHasBorderInFullScreen(true);
        }
#endif
    }
#endif
    connect(this, &QtSingleApplication::messageReceived, this, [=](const QString &message) {
        if (!message.isEmpty()) {
            this->openZimFile(message);
        }
    });

    restoreWindowState();
}

void KiwixApp::setupDirectoryMonitoring()
{
    QString monitorDir = m_settingsManager.getMonitorDir();
    QString downloadDir = m_settingsManager.getDownloadDir();
    auto dirList = QSet<QString>({monitorDir, downloadDir});
    mp_manager->setMonitoredDirectories(dirList);
}

KiwixApp::~KiwixApp()
{
    m_server.stop();
    if (mp_manager) {
        delete mp_manager;
    }
    if (mp_mainWindow) {
        delete mp_mainWindow;
    }
}

void KiwixApp::newTab()
{
    getTabWidget()->createNewTab(true, false);
    auto& searchBarLineEdit = getSearchBar().getLineEdit();
    searchBarLineEdit.setFocus(Qt::MouseFocusReason);
    searchBarLineEdit.clear();
    searchBarLineEdit.clearSuggestions();
    searchBarLineEdit.hideSuggestions();
}

QString KiwixApp::findLibraryDirectory()
{
    auto dataDir = getDataDirectory();
    if (isPortableMode())
        return dataDir;

    auto libraryFile = QFileInfo(dataDir, "library.xml");
    if (libraryFile.exists())
        return dataDir;

    // Check if this is a pre-release version with wrong directory.
    auto oldDataDir = QDir(dataDir);
    oldDataDir.cdUp();
    libraryFile = QFileInfo(oldDataDir, "library.xml");
    if (libraryFile.exists()) {
        // We have to move all zim files and xml file to the new dataDir
        for (auto& fileInfo: oldDataDir.entryInfoList({"*.zim", "library*.xml"})) {
            auto newFileInfo = QFileInfo(dataDir, fileInfo.fileName());
            QFile::rename(fileInfo.absoluteFilePath(), newFileInfo.absoluteFilePath());
        }
        // Aria2 stores information about the current download using absolute path.
        // Let's remove everything. User will lose its ongoing download but it should be pretty rare.
        for (auto& fileInfo: oldDataDir.entryInfoList({"*.aria2", "*.meta4", "kiwix.session"}))
            QFile::remove(fileInfo.absoluteFilePath());
    }

    // This is a first launch
    return dataDir;
}

void KiwixApp::restoreTabs()
{
    QStringList tabsToOpen = mp_session->value("reopenTabList").toStringList();

    /* Restart a new session to prevent duplicate records in openURL */
    saveListOfOpenTabs();
    if (m_settingsManager.getReopenTab())
    {
      for (const auto &zimUrl : tabsToOpen)
      {
        if (zimUrl == "SettingsTab")
          getTabWidget()->openOrSwitchToSettingsTab();
        else if (zimUrl.isEmpty())
          getTabWidget()->createNewTab(true, true);
        else
          openUrl(QUrl(zimUrl));
      }
    }

    /* Restore current tab index. */
    getTabWidget()->setCurrentIndex(mp_session->value("currentTabIndex", 0).toInt());
}

KiwixApp *KiwixApp::instance()
{
    return static_cast<KiwixApp*>(QApplication::instance());
}

QString gt(const QString &key) {
    return KiwixApp::instance()->getText(key);
}

void KiwixApp::openZimFile(const QString &zimfile)
{
    QString _zimfile;
    if (zimfile.isEmpty()) {
        QString importDir = mp_session->value("zim-import-dir").toString();
        if (importDir.isEmpty()) {
            importDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
            if (importDir.isEmpty()) { importDir = QDir::currentPath(); }
        }
        _zimfile = QFileDialog::getOpenFileName(getMainWindow(), gt("open-zim"), importDir, "ZIM Files (*.zim);;Split ZIM Files (*.zimaa)");

        if (_zimfile.isEmpty()) { return; }
        _zimfile = QDir::toNativeSeparators(_zimfile);
        QFileInfo fileInfo(_zimfile);
        mp_session->setValue("zim-import-dir", fileInfo.absolutePath());
    }
    QString zimId;
    const auto &validZimFile = zimfile.isEmpty() ? _zimfile : zimfile;
    try {
        zimId = m_library.openBookFromPath(validZimFile);
    } catch (const std::exception& e) {
        auto text = gt("zim-open-fail-text");
        text = text.replace("{{ZIM}}", validZimFile);
        showMessage(text, gt("zim-open-fail-title"), QMessageBox::Warning);
        return;
    }
    openUrl(QUrl("zim://"+zimId+".zim/"));
}

void KiwixApp::printPage()
{
    if(!getTabWidget()->currentZimView())
        return;

    QPrinter* printer = new QPrinter();
    QPrintDialog printDialog(printer, mp_mainWindow);
    printDialog.setStyle(nullptr);
    printDialog.setStyleSheet("");
    if (printDialog.exec() == QDialog::Accepted) {
        auto webview = getTabWidget()->currentWebView();
        if(!webview)
            return;

    const auto onPrintFinished = [=](bool success) {
        if (!success) {
            showMessage(gt("print-page-error"), gt("error-title"), QMessageBox::Critical);
        }
        delete printer;
    };
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        webview->page()->print(printer, onPrintFinished);
#else
        webview->print(printer);
        connect(webview, &QWebEngineView::printFinished, this, onPrintFinished);
#endif
    }
}

void KiwixApp::openUrl(const QString &url, bool newTab) {
    openUrl(QUrl(url), newTab);
}

void KiwixApp::openUrl(const QUrl &url, bool newTab) {
    getTabWidget()->openUrl(url, newTab);
}

void KiwixApp::openRandomUrl(bool newTab)
{
    auto zimId = getTabWidget()->currentZimId();
    if (zimId.isEmpty()) {
        return;
    }

    try {
        auto archive = m_library.getArchive(zimId);
        auto entry = archive->getRandomEntry();

        QUrl url;
        url.setScheme("zim");
        url.setHost(zimId + ".zim");
        url.setPath("/" + QString::fromStdString(entry.getPath()));
        openUrl(url, newTab);
    } catch (const zim::EntryNotFound& e) {
        showMessage(gt("random-article-error"), gt("error-title"), QMessageBox::Information);
    } catch (std::out_of_range& e) {
        showMessage(gt("error-archive"), gt("error-title"), QMessageBox::Information);
    }
}

void KiwixApp::showMessage(const QString &message, const QString &title, const enum QMessageBox::Icon &icon)
{
    QMessageBox msgBox(
        icon, //Icon
        title, //Title
        message, //Text
        QMessageBox::Ok //Buttons
    );
    msgBox.exec();
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

void KiwixApp::setMonitorDir(const QString &dir) {
    m_settingsManager.setMonitorDir(dir);
    setupDirectoryMonitoring();
}

#define CREATE_ACTION(ID, TEXT) \
    mpa_actions[ID] = new QAction(TEXT)
#define CREATE_ACTION_ICON(ID, ICON, TEXT) \
    mpa_actions[ID] = new QAction(QIcon(":/icons/" ICON ".svg"), TEXT);
#define SET_SHORTCUT(ID, TEXT, SHORTCUT) \
    mpa_actions[ID]->setShortcut(SHORTCUT); \
    mpa_actions[ID]->setToolTip(TEXT + " (" + QKeySequence(SHORTCUT).toString() + ")" )
#define CREATE_ACTION_SHORTCUT(ID, TEXT, SHORTCUT) \
    CREATE_ACTION(ID, TEXT); \
    SET_SHORTCUT(ID, TEXT, SHORTCUT)
#define CREATE_ACTION_ICON_SHORTCUT(ID, ICON, TEXT, SHORTCUT) \
    CREATE_ACTION_ICON(ID, ICON, TEXT) \
    SET_SHORTCUT(ID, TEXT, SHORTCUT)
#define CREATE_ACTION_ONOFF_ICON_SHORTCUT(ID, ON_ICON, OFF_ICON, TEXT, SHORTCUT) \
    CREATE_ACTION(ID, TEXT); \
    SET_SHORTCUT(ID, TEXT, SHORTCUT); \
    mpa_actions[ID]->setCheckable(true); \
    {   QIcon icon; \
        icon.addPixmap(QPixmap(":/icons/" ON_ICON ".svg"), QIcon::Normal, QIcon::On); \
        icon.addPixmap(QPixmap(":/icons/" OFF_ICON, ".svg"), QIcon::Normal, QIcon::Off); \
        mpa_actions[ID]->setIcon(icon); }
#define SET_SHORTCUTS(ID, TEXT, SHORTCUTS) \
    mpa_actions[ID]->setShortcuts(SHORTCUTS); \
    mpa_actions[ID]->setToolTip(TEXT + " (" + SHORTCUTS.first().toString() + ")" )
#define CREATE_ACTION_SHORTCUTS(ID, TEXT, SHORTCUTS) \
    CREATE_ACTION(ID, TEXT); \
    SET_SHORTCUTS(ID, TEXT, SHORTCUTS)
#define CREATE_ACTION_ICON_SHORTCUTS(ID, ICON, TEXT, SHORTCUTS) \
    CREATE_ACTION_ICON(ID, ICON, TEXT) \
    SET_SHORTCUTS(ID, TEXT, SHORTCUTS)
#define HIDE_ACTION(ID) mpa_actions[ID]->setVisible(false)
#define DISABLE_ACTION(ID) mpa_actions[ID]->setDisabled(true)

void KiwixApp::createActions()
{
    CREATE_ACTION_ICON_SHORTCUT(KiwixServeAction, "share", gt("local-kiwix-server"), QKeySequence(Qt::CTRL | Qt::Key_I));

    CREATE_ACTION_ICON_SHORTCUT(RandomArticleAction, "random", gt("random-article"), QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(mpa_actions[RandomArticleAction], &QAction::triggered,
            this, [=]() { this->openRandomUrl(false); });

    CREATE_ACTION_ICON_SHORTCUT(OpenHomePageAction, "home-button", gt("home-page"), QKeySequence(Qt::ALT | Qt::Key_Home));

    if (QGuiApplication::isLeftToRight()) {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-left", gt("back"), QKeySequence(Qt::ALT | Qt::Key_Left));
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-right", gt("forward"), QKeySequence(Qt::ALT | Qt::Key_Right));
      CREATE_ACTION_ICON(ScrollNextTabAction, "caret-right-solid", gt("scroll-next-tab"));
      CREATE_ACTION_ICON(ScrollPreviousTabAction, "caret-left-solid", gt("scroll-previous-tab"));
      CREATE_ACTION_SHORTCUT(NextTabAction, gt("next-tab"), QKeySequence(Qt::CTRL | Qt::Key_Tab));
      CREATE_ACTION_SHORTCUT(NextTabAction, gt("next-tab"), QKeySequence(Qt::CTRL | Qt::Key_PageDown ));
      CREATE_ACTION_SHORTCUT(PreviousTabAction, gt("previous-tab"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));
      CREATE_ACTION_SHORTCUT(PreviousTabAction, gt("previous-tab"), QKeySequence(Qt::CTRL | Qt::Key_PageUp ));
    } else {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-right", gt("back"), QKeySequence(Qt::ALT | Qt::Key_Right));
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-left", gt("forward"), QKeySequence(Qt::ALT | Qt::Key_Left));
      CREATE_ACTION_ICON(ScrollNextTabAction, "caret-left-solid", gt("scroll-next-tab"));
      CREATE_ACTION_ICON(ScrollPreviousTabAction, "caret-right-solid", gt("scroll-previous-tab"));
      CREATE_ACTION_SHORTCUT(NextTabAction, gt("next-tab"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));
      CREATE_ACTION_SHORTCUT(NextTabAction, gt("next-tab"), QKeySequence(Qt::CTRL | Qt::Key_PageUp));
      CREATE_ACTION_SHORTCUT(PreviousTabAction, gt("previous-tab"), QKeySequence(Qt::CTRL | Qt::Key_Tab));
      CREATE_ACTION_SHORTCUT(PreviousTabAction, gt("previous-tab"), QKeySequence(Qt::CTRL | Qt::Key_PageDown ));
    }
    DISABLE_ACTION(HistoryBackAction);
    DISABLE_ACTION(HistoryForwardAction);

    CREATE_ACTION_SHORTCUT(ReadArticleAction, gt("read-article"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_A));
    CREATE_ACTION_SHORTCUT(ReadTextAction, gt("read-text"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_S));
    CREATE_ACTION_SHORTCUT(ReadStopAction, gt("read-stop"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_X));
    CREATE_ACTION_SHORTCUT(ToggleTTSLanguageAction, gt("select-read-language"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_L));
    CREATE_ACTION_SHORTCUT(ToggleTTSVoiceAction, gt("select-read-voice"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_V));
    CREATE_ACTION_SHORTCUT(IncreaseTTSSpeedAction, gt("increase-tts-speed"), QKeySequence(Qt::Key_Greater));
    CREATE_ACTION_SHORTCUT(DecreaseTTSSpeedAction, gt("decrease-tts-speed"), QKeySequence(Qt::Key_Less));
    mpa_actions[ToggleTTSLanguageAction]->setCheckable(true);
    mpa_actions[ToggleTTSVoiceAction]->setCheckable(true);

    CREATE_ACTION_ICON_SHORTCUT(PrintAction, "print", gt("print"), QKeySequence::Print);
    connect(mpa_actions[PrintAction], &QAction::triggered,
            this, &KiwixApp::printPage);

    CREATE_ACTION_ICON_SHORTCUT(NewTabAction,"new-tab-icon", gt("new-tab"), QKeySequence::AddTab);

    CREATE_ACTION_SHORTCUTS(CloseCurrentTabAction, gt("close-tab"), QList<QKeySequence>({QKeySequence(Qt::CTRL | Qt::Key_F4), QKeySequence(Qt::CTRL | Qt::Key_W)}));

    CREATE_ACTION_SHORTCUT(ReopenClosedTabAction, gt("reopen-closed-tab"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    HIDE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION_SHORTCUT(BrowseLibraryAction, gt("browse-library"), QKeySequence(Qt::CTRL | Qt::Key_E));
    HIDE_ACTION(BrowseLibraryAction);

    CREATE_ACTION_ICON_SHORTCUT(OpenFileAction, "open-file", gt("open-file"), QKeySequence::Open);
    connect(mpa_actions[OpenFileAction], &QAction::triggered,
            this, [=]() { openZimFile(); });

    CREATE_ACTION(OpenRecentAction, gt("open-recent"));
    HIDE_ACTION(OpenRecentAction);

    CREATE_ACTION_SHORTCUT(SavePageAsAction, gt("save-page-as"), QKeySequence::Save);

    CREATE_ACTION_SHORTCUTS(SearchArticleAction, gt("search-article"), QList<QKeySequence>({QKeySequence(Qt::Key_F6), QKeySequence(Qt::CTRL | Qt::Key_L), QKeySequence(Qt::ALT | Qt::Key_D)}));

    CREATE_ACTION_SHORTCUT(SearchLibraryAction, gt("search-in-library"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, gt("find-in-page"));
    mpa_actions[FindInPageAction]->setShortcuts({QKeySequence::Find, Qt::Key_F3});
    connect(mpa_actions[FindInPageAction], &QAction::triggered,
            this, [=]() { getTabWidget()->openFindInPageBar(); });

    const auto fullScreenKeySeq = QKeySequence(QKeySequence::FullScreen).isEmpty()
                                ? QKeySequence(Qt::Key_F11)
                                : QKeySequence(QKeySequence::FullScreen);
    CREATE_ACTION_ICON_SHORTCUT(ToggleFullscreenAction, "full-screen-enter", gt("set-fullscreen"), fullScreenKeySeq);
    connect(mpa_actions[ToggleFullscreenAction], &QAction::toggled,
            this, [=](bool checked) {
        auto action = mpa_actions[ToggleFullscreenAction];
        action->setIcon(
            QIcon(checked ? ":/icons/full-screen-exit.svg" : ":/icons/full-screen-enter.svg"));
        action->setText(checked ? gt("quit-fullscreen") : gt("set-fullscreen"));
    });
    mpa_actions[ToggleFullscreenAction]->setCheckable(true);

    CREATE_ACTION_ICON_SHORTCUT(ToggleTOCAction, "toc", gt("table-of-content"), QKeySequence(Qt::CTRL | Qt::Key_M));
    mpa_actions[ToggleTOCAction]->setCheckable(true);

    CREATE_ACTION_ICON_SHORTCUT(OpenMultiZimAction, "filter", gt("search-options"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));

    CREATE_ACTION_ONOFF_ICON_SHORTCUT(ToggleReadingListAction, "reading-list-active", "reading-list", gt("reading-list"), QKeySequence(Qt::CTRL | Qt::Key_B));

    CREATE_ACTION(ExportReadingListAction, gt("export-reading-list"));

    CREATE_ACTION(ImportReadingListAction, gt("import-reading-list"));

    CREATE_ACTION_ONOFF_ICON_SHORTCUT(ToggleAddBookmarkAction, "star-active", "star", gt("add-bookmark"), QKeySequence(Qt::CTRL | Qt::Key_D));

    CREATE_ACTION_SHORTCUTS(ZoomInAction, gt("zoom-in"), QList<QKeySequence>({QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)}));

    CREATE_ACTION_SHORTCUT(ZoomOutAction, gt("zoom-out"), QKeySequence::ZoomOut);

    CREATE_ACTION_SHORTCUT(ZoomResetAction, gt("zoom-reset"), QKeySequence(Qt::CTRL | Qt::Key_0));

    CREATE_ACTION_ICON_SHORTCUT(HelpAction, "help", gt("help"), QKeySequence::HelpContents);
    HIDE_ACTION(HelpAction);

    CREATE_ACTION_ICON_SHORTCUT(CheckUpdatesAction, "update", gt("check-update-title"), QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(mpa_actions[CheckUpdatesAction], &QAction::triggered,
            this, &KiwixApp::checkForUpdates);

    CREATE_ACTION(FeedbackAction, gt("feedback"));
    HIDE_ACTION(FeedbackAction);

    CREATE_ACTION(ReportBugAction, gt("report-a-bug"));
    HIDE_ACTION(ReportBugAction);

    CREATE_ACTION(RequestFeatureAction, gt("request-a-feature"));
    HIDE_ACTION(RequestFeatureAction);

    CREATE_ACTION(AboutAction, gt("about-kiwix"));

    CREATE_ACTION_ICON_SHORTCUT(SettingAction, "settings", gt("settings"),  QKeySequence(Qt::Key_F12));

    CREATE_ACTION_ICON_SHORTCUT(DonateAction, "donate", gt("donate-to-support-kiwix"),  QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));

    CREATE_ACTION_ICON_SHORTCUT(ExitAction, "exit", gt("exit"), QKeySequence::Quit);
}

void KiwixApp::postInit() {
    connect(getTabWidget(), &TabBar::tabDisplayed,
            this, &KiwixApp::handleItemsState);
    emit(m_library.booksChanged());
    connect(&m_library, &Library::booksChanged, this, &KiwixApp::updateNameMapper);
    handleItemsState(TabType::LibraryTab);
}

void KiwixApp::handleItemsState(TabType tabType)
{
    auto libraryOrSettingsTab = (tabType == TabType::LibraryTab || tabType == TabType::SettingsTab);
    auto notBookmarkableTab = libraryOrSettingsTab || getTabWidget()->currentArticleUrl().isEmpty();
    auto hasZimFile = !getTabWidget()->currentZimId().isEmpty();
    auto app = KiwixApp::instance();

    // Navigation actions
    app->getAction(KiwixApp::ToggleTOCAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::OpenHomePageAction)->setDisabled(!hasZimFile);
    app->getAction(KiwixApp::RandomArticleAction)->setDisabled(!hasZimFile);

    // Reading & bookmarks
    app->getAction(KiwixApp::ToggleReadingListAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ToggleAddBookmarkAction)->setDisabled(notBookmarkableTab);
    app->getAction(KiwixApp::ReadArticleAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ReadTextAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ReadStopAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ToggleTTSLanguageAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ToggleTTSVoiceAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::IncreaseTTSSpeedAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::DecreaseTTSSpeedAction)->setDisabled(libraryOrSettingsTab);

    // Search & zoom
    app->getAction(KiwixApp::FindInPageAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ZoomInAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ZoomOutAction)->setDisabled(libraryOrSettingsTab);
    app->getAction(KiwixApp::ZoomResetAction)->setDisabled(libraryOrSettingsTab);

    // File operations
    app->getAction(KiwixApp::PrintAction)->setDisabled(!hasZimFile);
    app->getAction(KiwixApp::SavePageAsAction)->setDisabled(!hasZimFile);


    /* Non-Zim tabs are not bookmarkable therefore never in reading list. */
    if (notBookmarkableTab)
        app->getAction(KiwixApp::ToggleAddBookmarkAction)->setChecked(false);
}

void KiwixApp::updateNameMapper()
{
  mp_nameMapper->update();
}

void KiwixApp::printVersions(std::ostream& out) {
  out << version.toStdString() << std::endl;
  out << "+ libqt (compile time) " << QT_VERSION_STR << std::endl;
  out << "+ libqt (run time) " << qVersion() << std::endl << std::endl;
  kiwix::printVersions(out);
  out << std::endl;
  zim::printVersions(out);
}

QString KiwixApp::parseStyleFromFile(QString filePath)
{
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    file.close();
    return styleSheet;
}

void KiwixApp::saveListOfOpenTabs()
{
  return mp_session->setValue("reopenTabList", getTabWidget()->getTabUrls());
}

void KiwixApp::saveWindowState()
{
  mp_session->setValue("geometry", getMainWindow()->saveGeometry());
  mp_session->setValue("windowState", getMainWindow()->saveState());
}

void KiwixApp::saveVoiceName(const QString& langName, const QString& voiceName)
{
  mp_session->setValue("voice/" + langName, voiceName);
}

void KiwixApp::saveTtsSpeed(const QString& langName, double speed)
{
    mp_session->setValue("speed/" + langName, speed);
}

void KiwixApp::restoreWindowState()
{
  getMainWindow()->restoreGeometry(mp_session->value("geometry").toByteArray());
  getMainWindow()->restoreState(mp_session->value("windowState").toByteArray());
}

void KiwixApp::saveCurrentTabIndex()
{
  return mp_session->setValue("currentTabIndex", getTabWidget()->currentIndex());
}

void KiwixApp::savePrevSaveDir(const QString &prevSaveDir)
{
  mp_session->setValue("prevSaveDir", prevSaveDir);
}

QString KiwixApp::getSavedVoiceName(const QString& langName) const
{
  return mp_session->value("voice/" + langName, "").toString();
}

double KiwixApp::getSavedTtsSpeed(const QString& langName) const
{
    return mp_session->value("speed/" + langName, 1.0).toDouble(); // Default: 1.0 (normal speed)
}

QString KiwixApp::getPrevSaveDir() const
{
  QString prevSaveDir = mp_session->value("prevSaveDir", DEFAULT_SAVE_DIR).toString();
  QDir dir(prevSaveDir);
  return dir.exists() ? prevSaveDir : DEFAULT_SAVE_DIR;
}

void KiwixApp::checkForUpdates()
{
    if (!mp_versionChecker) {
        mp_versionChecker = std::make_unique<VersionChecker>();
        connect(mp_versionChecker.get(), &VersionChecker::updateAvailable,
                this, &KiwixApp::handleUpdateCheckResult);
        connect(mp_versionChecker.get(), &VersionChecker::noUpdateAvailable,
                this, &KiwixApp::handleNoUpdateAvailable);
        connect(mp_versionChecker.get(), &VersionChecker::checkFailed,
                this, &KiwixApp::handleUpdateCheckFailed);
    }
    mp_versionChecker->checkForUpdates();
}

void KiwixApp::handleUpdateCheckResult(const QString& latestVersion)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(gt("update-available-title"));
    msgBox.setText(gt("update-available").replace("{{VERSION}}", latestVersion) + 
                  "\n" + gt("current-version").replace("{{VERSION}}", version));
    msgBox.setInformativeText(gt("update-available-message"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void KiwixApp::handleNoUpdateAvailable()
{
    QMessageBox::information(nullptr,
                           gt("check-update-title"),
                           gt("no-update-available") + "\n" +
                           gt("current-version").replace("{{VERSION}}", version));
}

void KiwixApp::handleUpdateCheckFailed(const QString& error)
{
    QMessageBox::warning(nullptr,
                        gt("check-update-title"),
                        gt("update-check-failed").replace("{{ERROR}}", error));
}
