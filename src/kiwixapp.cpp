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
#ifdef Q_OS_WIN
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif

////////////////////////////////////////////////////////////////////////////////
// KiwixApp
////////////////////////////////////////////////////////////////////////////////

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QtSingleApplication("kiwix-desktop", argc, argv),
      m_profile(),
      m_libraryDirectory(findLibraryDirectory()),
      m_library(m_libraryDirectory),
      mp_downloader(nullptr),
      mp_manager(nullptr),
      mp_mainWindow(nullptr),
      mp_nameMapper(std::make_shared<kiwix::UpdatableNameMapper>(m_library.getKiwixLibrary(), false)),
      m_server(m_library.getKiwixLibrary(), mp_nameMapper)
{
    try {
        m_translation.setTranslation(QLocale());
    } catch (std::exception& e) {
        QMessageBox::critical(nullptr, "Translation error", e.what());
        return;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_qtTranslator.load(QLocale(), "qt", "_",
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#else
    m_qtTranslator.load(QLocale(), "qt", "_",
                        QLibraryInfo::path(QLibraryInfo::TranslationsPath));
#endif
    installTranslator(&m_qtTranslator);

    m_appTranslator.load(QLocale(), "kiwix-desktop", "_", ":/i18n/");
    installTranslator(&m_appTranslator);

    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawkb.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawkl.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawksb.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawksl.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Selawik/selawk.ttf");
    setFont(QFont("Selawik"));
}

void KiwixApp::init()
{
    try {
        mp_downloader = new kiwix::Downloader();
    } catch (std::exception& e) {
        QMessageBox::critical(nullptr, gt("error-downloader-window-title"),
        gt("error-downloader-launch-message") + "<br><br>" + e.what());
    }
    mp_manager = new ContentManager(&m_library, mp_downloader);
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
        QWindowsWindowFunctions::setHasBorderInFullScreen(window, true);
    }
#endif
    connect(this, &QtSingleApplication::messageReceived, this, [=](const QString &message) {
        if (!message.isEmpty()) {
            this->openZimFile(message);
        }
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [=](QString monitorDir) {
        m_library.asyncUpdateFromDir(monitorDir);
    });
    QString monitorDir = m_settingsManager.getMonitorDir();
    QString downloadDir = m_settingsManager.getDownloadDir();
    auto dirList = QSet<QString>({monitorDir, downloadDir});
    for (auto dir : dirList) {
        if (dir != "") {
            m_library.setMonitorDirZims(dir, m_library.getLibraryZimsFromDir(dir));
            m_watcher.addPath(dir);
            m_library.asyncUpdateFromDir(dir);
        }
    }
}

KiwixApp::~KiwixApp()
{
    m_server.stop();
    if (mp_downloader) {
        mp_downloader->close();
        delete mp_downloader;
    }
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
    auto& searchBar = mp_mainWindow->getTopWidget()->getSearchBar();
    searchBar.setFocus(Qt::MouseFocusReason);
    searchBar.clear();
    searchBar.clearSuggestions();
    searchBar.hideSuggestions();
}

QString KiwixApp::findLibraryDirectory()
{
  // Check for library.xml in the same directory than the executable (portable kiwix-desktop)
  auto currentDataDir = QString::fromStdString(kiwix::removeLastPathElement(kiwix::getExecutablePath()));
  auto libraryFile = QFileInfo(currentDataDir, "library.xml");
  if (libraryFile.exists())
    return currentDataDir;

  // Check for default dataDirectory.
  currentDataDir = QString::fromStdString(kiwix::getDataDirectory());
  libraryFile = QFileInfo(currentDataDir, "library.xml");
  if (libraryFile.exists())
    return currentDataDir;

  // There is no library.xml in default dataDirectory.
  // Either, it is a first launch, or user used a pre-release version with wrong directory.
  // Let's try to move data from old directory to new one if needed.
  auto oldDataDir = QDir(currentDataDir);
  oldDataDir.cdUp();
  libraryFile = QFileInfo(oldDataDir, "library.xml");
  if (libraryFile.exists()) {
    // We have to move all zims file and xml file to the new dataDir
    for (auto& fileInfo: oldDataDir.entryInfoList({"*.zim", "library*.xml"})) {
      auto newFileInfo = QFileInfo(currentDataDir, fileInfo.fileName());
      QFile::rename(fileInfo.absoluteFilePath(), newFileInfo.absoluteFilePath());
    }
    // Aria2 store informations about the current download using absolute path.
    // Let's remove everything. User will loose its ongoing download but it should be pretty rare.
    for (auto& fileInfo: oldDataDir.entryInfoList({"*.aria2", "*.meta4", "kiwix.session"})) {
      QFile::remove(fileInfo.absoluteFilePath());
    }
  }

  return currentDataDir;
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
        _zimfile = QFileDialog::getOpenFileName(
                    getMainWindow(),
                    gt("open-zim"),
                    QString(),
                    "ZIM Files (*.zim);;Splitted ZIM Files (*.zimaa)");

        if (_zimfile.isEmpty()) {
            return;
        }
        _zimfile = QDir::toNativeSeparators(_zimfile);
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
    m_library.setMonitorDirZims(dir, QStringList());
    for (auto path : m_watcher.directories()) {
        m_watcher.removePath(path);
    }
    if (dir != "") {
        m_watcher.addPath(dir);
        m_watcher.addPath(m_settingsManager.getDownloadDir());
        m_library.asyncUpdateFromDir(dir);
    }
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

    CREATE_ACTION_SHORTCUT(OpenHomePageAction, gt("home-page"), QKeySequence(Qt::ALT | Qt::Key_Home));

    if (QGuiApplication::isLeftToRight()) {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-left", gt("back"), QKeySequence(Qt::ALT | Qt::Key_Left));
    } else {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-right", gt("back"), QKeySequence(Qt::ALT | Qt::Key_Right));
    }
    DISABLE_ACTION(HistoryBackAction);

    if (QGuiApplication::isLeftToRight()) {
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-right", gt("forward"), QKeySequence(Qt::ALT | Qt::Key_Right));
    } else {
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-left", gt("forward"), QKeySequence(Qt::ALT | Qt::Key_Left));
    }
    DISABLE_ACTION(HistoryForwardAction);

    CREATE_ACTION_ICON_SHORTCUT(PrintAction, "print", gt("print"), QKeySequence::Print);
    connect(mpa_actions[PrintAction], &QAction::triggered,
            this, &KiwixApp::printPage);

    CREATE_ACTION_ICON_SHORTCUT(NewTabAction,"new-tab-icon", gt("new-tab"), QKeySequence::AddTab);

    CREATE_ACTION_ICON_SHORTCUTS(CloseTabAction, "close", gt("close-tab"), QList<QKeySequence>({QKeySequence(Qt::CTRL | Qt::Key_F4), QKeySequence(Qt::CTRL | Qt::Key_W)}));
    mpa_actions[CloseTabAction]->setIconVisibleInMenu(false);

    CREATE_ACTION_SHORTCUT(ReopenClosedTabAction, gt("reopen-closed-tab"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    HIDE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION_SHORTCUT(BrowseLibraryAction, gt("browse-library"), QKeySequence(Qt::CTRL | Qt::Key_E));
    HIDE_ACTION(BrowseLibraryAction);

    CREATE_ACTION_ICON_SHORTCUT(OpenFileAction, "open-file", gt("open-file"), QKeySequence::Open);
    connect(mpa_actions[OpenFileAction], &QAction::triggered,
            this, [=]() { openZimFile(); });

    CREATE_ACTION(OpenRecentAction, gt("open-recent"));
    HIDE_ACTION(OpenRecentAction);

    /* TODO See https://github.com/kiwix/kiwix-desktop/issues/77
    CREATE_ACTION(SavePageAsAction, tr("Save page as ..."));
    // SET_SHORTCUT(SavePageAsAction, QKeySequence::SaveAs);
    HIDE_ACTION(SavePageAsAction);
    */

    CREATE_ACTION_SHORTCUT(SearchArticleAction, gt("search-article"), QKeySequence(Qt::CTRL | Qt::Key_L));
    HIDE_ACTION(SearchArticleAction);

    CREATE_ACTION_SHORTCUT(SearchLibraryAction, gt("search-in-library"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, gt("find-in-page"));
    mpa_actions[FindInPageAction]->setShortcuts({QKeySequence::Find, Qt::Key_F3});
    connect(mpa_actions[FindInPageAction], &QAction::triggered,
            this, [=]() { getTabWidget()->openFindInPageBar(); });

    CREATE_ACTION_ICON_SHORTCUT(ToggleFullscreenAction, "full-screen-enter", gt("set-fullscreen"),  QKeySequence::FullScreen);
    connect(mpa_actions[ToggleFullscreenAction], &QAction::toggled,
            this, [=](bool checked) {
        auto action = mpa_actions[ToggleFullscreenAction];
        action->setIcon(
            QIcon(checked ? ":/icons/full-screen-exit.svg" : ":/icons/full-screen-enter.svg"));
        action->setText(checked ? gt("quit-fullscreen") : gt("set-fullscreen"));
    });
    mpa_actions[ToggleFullscreenAction]->setCheckable(true);

    CREATE_ACTION_SHORTCUT(ToggleTOCAction, gt("table-of-content"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_1));
    HIDE_ACTION(ToggleTOCAction);

    CREATE_ACTION_ONOFF_ICON_SHORTCUT(ToggleReadingListAction, "reading-list-active", "reading-list", gt("reading-list"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_2));

    CREATE_ACTION_SHORTCUTS(ZoomInAction, gt("zoom-in"), QList<QKeySequence>({QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)}));

    CREATE_ACTION_SHORTCUT(ZoomOutAction, gt("zoom-out"), QKeySequence::ZoomOut);

    CREATE_ACTION_SHORTCUT(ZoomResetAction, gt("zoom-reset"), QKeySequence(Qt::CTRL | Qt::Key_0));

    CREATE_ACTION_SHORTCUT(NextTabAction, gt("next-tab"), QKeySequence(Qt::CTRL | Qt::Key_Tab));

    CREATE_ACTION_SHORTCUT(PreviousTabAction, gt("previous-tab"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));

    CREATE_ACTION_SHORTCUT(HelpAction, gt("help"), QKeySequence::HelpContents);
    HIDE_ACTION(HelpAction);

    CREATE_ACTION(FeedbackAction, gt("feedback"));
    HIDE_ACTION(FeedbackAction);

    CREATE_ACTION(ReportBugAction, gt("report-a-bug"));
    HIDE_ACTION(ReportBugAction);

    CREATE_ACTION(RequestFeatureAction, gt("request-a-feature"));
    HIDE_ACTION(RequestFeatureAction);

    CREATE_ACTION(AboutAction, gt("about-kiwix"));

    CREATE_ACTION_ICON_SHORTCUT(SettingAction, "settings", gt("settings"),  QKeySequence(Qt::Key_F12));

    CREATE_ACTION_ICON_SHORTCUT(DonateAction, "donate", gt("donate-to-support-kiwix"),  QKeySequence(Qt::CTRL | Qt::Key_D));

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
    auto libraryOrSettingsTab =  (tabType == TabType::LibraryTab || tabType == TabType::SettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::ToggleReadingListAction)->setDisabled(libraryOrSettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::FindInPageAction)->setDisabled(libraryOrSettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::ZoomInAction)->setDisabled(libraryOrSettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::ZoomOutAction)->setDisabled(libraryOrSettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::ZoomResetAction)->setDisabled(libraryOrSettingsTab);
    KiwixApp::instance()->getAction(KiwixApp::RandomArticleAction)->setDisabled(libraryOrSettingsTab);
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
