#include "kiwixapp.h"
#include "static_content.h"
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
      m_nameMapper(m_library.getKiwixLibrary(), false),
      m_server(&m_library.getKiwixLibrary(), &m_nameMapper)
{
    try {
        m_translation.setTranslation(QLocale());
    } catch (exception& e) {
        QMessageBox::critical(nullptr, "Translation error", e.what());
        return;
    }
    m_qtTranslator.load(QLocale(), "qt", "_",
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
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
    } catch (exception& e) {
        QMessageBox::critical(nullptr, gt("error-downloader-window-title"),
        gt("error-downloader-launch-message") + "<br><br>" + e.what());
    }
    mp_manager = new ContentManager(&m_library, mp_downloader);

    initStaticContent();

    auto icon = QIcon();
    icon.addFile(":/icons/kiwix-app-icons-square.svg");
    setWindowIcon(icon);

    setApplicationName("Kiwix");
    setDesktopFileName("kiwix.desktop");

    QFile styleFile(":/css/style.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    QString style(byteContent);
    setStyleSheet(style);


    createAction();
    mp_mainWindow = new MainWindow;
    mp_tabWidget = mp_mainWindow->getTabBar();
    mp_tabWidget->setContentManagerView(mp_manager->getView());
    mp_tabWidget->setNewTabButton();
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
                    "ZimFile (*.zim*)");

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
        showMessage("Cannot open " + validZimFile + ": \n" + e.what());
        return;
    }
    openUrl(QUrl("zim://"+zimId+".zim/"));
}

void KiwixApp::printPage()
{
    if(!mp_tabWidget->currentZimView())
        return;
    QPrinter* printer = new QPrinter();
    QPrintDialog printDialog(printer, mp_mainWindow);
    printDialog.setStyle(nullptr);
    printDialog.setStyleSheet("");
    if (printDialog.exec() == QDialog::Accepted) {
        auto webview = mp_tabWidget->currentWebView();
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

void KiwixApp::openRandomUrl(bool newTab)
{
    auto zimId = mp_tabWidget->currentZimId();
    if (zimId.isEmpty()) {
        return;
    }
    auto reader = m_library.getReader(zimId);
    try {
        auto entry = reader->getRandomPage();

        QUrl url;
        url.setScheme("zim");
        url.setHost(zimId + ".zim");
        url.setPath("/" + QString::fromStdString(entry.getPath()));
        openUrl(url, newTab);
    } catch ( const kiwix::NoEntry& ) {
        showMessage(gt("random-article-error"));
    }
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

#define CREATE_ACTION(ID, TEXT) \
    mpa_actions[ID] = new QAction(TEXT)
#define SET_SHORTCUT(ID, TEXT, SHORTCUT) \
    mpa_actions[ID]->setShortcut(SHORTCUT); \
    mpa_actions[ID]->setToolTip(TEXT + " (" + QKeySequence(SHORTCUT).toString() + ")" )
#define CREATE_ACTION_SHORTCUT(ID, TEXT, SHORTCUT) \
    CREATE_ACTION(ID, TEXT); \
    SET_SHORTCUT(ID, TEXT, SHORTCUT)
#define CREATE_ACTION_ICON_SHORTCUT(ID, ICON, TEXT, SHORTCUT) \
    mpa_actions[ID] = new QAction(QIcon(":/icons/" ICON ".svg"), TEXT); \
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
#define HIDE_ACTION(ID) mpa_actions[ID]->setVisible(false)
#define DISABLE_ACTION(ID) mpa_actions[ID]->setDisabled(true)

void KiwixApp::createAction()
{
    CREATE_ACTION_ICON_SHORTCUT(KiwixServeAction, "share", gt("local-kiwix-server"), QKeySequence(Qt::CTRL+Qt::Key_I));

    CREATE_ACTION_ICON_SHORTCUT(RandomArticleAction, "random", gt("random-article"), QKeySequence(Qt::CTRL+Qt::Key_R));
    connect(mpa_actions[RandomArticleAction], &QAction::triggered,
            this, [=]() { this->openRandomUrl(false); });

    CREATE_ACTION_SHORTCUT(OpenHomePageAction, gt("home-page"), QKeySequence(Qt::ALT + Qt::Key_Home));

    if (QGuiApplication::isLeftToRight()) {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-left", gt("back"), QKeySequence(Qt::ALT + Qt::Key_Left));
    } else {
      CREATE_ACTION_ICON_SHORTCUT(HistoryBackAction, "history-right", gt("back"), QKeySequence(Qt::ALT + Qt::Key_Right));
    }
    DISABLE_ACTION(HistoryBackAction);

    if (QGuiApplication::isLeftToRight()) {
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-right", gt("forward"), QKeySequence(Qt::ALT + Qt::Key_Right));
    } else {
      CREATE_ACTION_ICON_SHORTCUT(HistoryForwardAction, "history-left", gt("forward"), QKeySequence(Qt::ALT + Qt::Key_Left));
    }
    DISABLE_ACTION(HistoryForwardAction);

    CREATE_ACTION_ICON_SHORTCUT(PrintAction, "print", gt("print"), QKeySequence::Print);
    connect(mpa_actions[PrintAction], &QAction::triggered,
            this, &KiwixApp::printPage);

    CREATE_ACTION_ICON_SHORTCUT(NewTabAction,"new-tab-icon", gt("new-tab"), QKeySequence::AddTab);

    CREATE_ACTION_ICON_SHORTCUT(CloseTabAction, "close", gt("close-tab"), QKeySequence::Close);
    mpa_actions[CloseTabAction]->setIconVisibleInMenu(false);

    CREATE_ACTION_SHORTCUT(ReopenClosedTabAction, gt("reopen-closed-tab"), QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_T));
    HIDE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION_SHORTCUT(BrowseLibraryAction, gt("browse-library"), QKeySequence(Qt::CTRL+Qt::Key_E));
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

    CREATE_ACTION_SHORTCUT(SearchArticleAction, gt("search-article"), QKeySequence(Qt::CTRL+Qt::Key_L));
    HIDE_ACTION(SearchArticleAction);

    CREATE_ACTION_SHORTCUT(SearchLibraryAction, gt("search-in-library"), QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, gt("find-in-page"));
    mpa_actions[FindInPageAction]->setShortcuts({QKeySequence::Find, Qt::Key_F3});
    connect(mpa_actions[FindInPageAction], &QAction::triggered,
            this, [=]() { mp_tabWidget->openFindInPageBar(); });

    CREATE_ACTION_ICON_SHORTCUT(ToggleFullscreenAction, "full-screen-enter", gt("set-fullscreen"),  QKeySequence::FullScreen);
    connect(mpa_actions[ToggleFullscreenAction], &QAction::toggled,
            this, [=](bool checked) {
        auto action = mpa_actions[ToggleFullscreenAction];
        action->setIcon(
            QIcon(checked ? ":/icons/full-screen-exit.svg" : ":/icons/full-screen-enter.svg"));
        action->setText(checked ? gt("quit-fullscreen") : gt("set-fullscreen"));
    });
    mpa_actions[ToggleFullscreenAction]->setCheckable(true);

    CREATE_ACTION_SHORTCUT(ToggleTOCAction, gt("table-of-content"), QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_1));
    HIDE_ACTION(ToggleTOCAction);

    CREATE_ACTION_ONOFF_ICON_SHORTCUT(ToggleReadingListAction, "reading-list-active", "reading-list", gt("reading-list"), QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_2));

    CREATE_ACTION_SHORTCUTS(ZoomInAction, gt("zoom-in"), QList<QKeySequence>({QKeySequence::ZoomIn, QKeySequence(Qt::CTRL+Qt::Key_Equal)}));

    CREATE_ACTION_SHORTCUT(ZoomOutAction, gt("zoom-out"), QKeySequence::ZoomOut);

    CREATE_ACTION_SHORTCUT(ZoomResetAction, gt("zoom-reset"), QKeySequence(Qt::CTRL+Qt::Key_0));

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

    CREATE_ACTION_ICON_SHORTCUT(DonateAction, "donate", gt("donate-to-support-kiwix"),  QKeySequence(Qt::CTRL+Qt::Key_D));

    CREATE_ACTION_ICON_SHORTCUT(ExitAction, "exit", gt("exit"), QKeySequence::Quit);
}

void KiwixApp::postInit() {
    connect(mp_tabWidget, &TabBar::webActionEnabledChanged,
            mp_mainWindow->getTopWidget(), &TopWidget::handleWebActionEnabledChanged);
    connect(mp_tabWidget, &TabBar::currentTitleChanged, this,
            [=](const QString& title) { emit currentTitleChanged(title); });
    connect(mp_tabWidget, &TabBar::libraryPageDisplayed,
            this, &KiwixApp::disableItemsOnLibraryPage);
    emit(m_library.booksChanged());
    connect(&m_library, &Library::booksChanged, this, &KiwixApp::updateNameMapper);
    disableItemsOnLibraryPage(true);
}

void KiwixApp::disableItemsOnLibraryPage(bool libraryDisplayed)
{
    KiwixApp::instance()->getAction(KiwixApp::ToggleReadingListAction)->setDisabled(libraryDisplayed);
    KiwixApp::instance()->getAction(KiwixApp::FindInPageAction)->setDisabled(libraryDisplayed);
    KiwixApp::instance()->getAction(KiwixApp::ZoomInAction)->setDisabled(libraryDisplayed);
    KiwixApp::instance()->getAction(KiwixApp::ZoomOutAction)->setDisabled(libraryDisplayed);
    KiwixApp::instance()->getAction(KiwixApp::ZoomResetAction)->setDisabled(libraryDisplayed);
}

void KiwixApp::updateNameMapper()
{
  m_nameMapper.update();
}

void KiwixApp::printVersions(std::ostream& out) {
  out << version.toStdString() << std::endl;
  out << "+ libqt (compile time) " << QT_VERSION_STR << std::endl;
  out << "+ libqt (run time) " << qVersion() << std::endl << std::endl;
  kiwix::printVersions(out);
  out << std::endl;
  zim::printVersions(out);
}
