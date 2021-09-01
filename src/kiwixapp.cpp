#include "kiwixapp.h"
#include "static_content.h"
#include "zim/error.h"
#include "kiwix/tools.h"

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

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QtSingleApplication("kiwix-desktop", argc, argv),
      m_settingsManager(),
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
    qInfo() << "Compiled with Qt Version " << QT_VERSION_STR;
    qInfo() << "Runtime Qt Version " << qVersion();
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
    mp_mainWindow->getSideContentManager()->setContentManager(mp_manager);
    setSideBar(CONTENTMANAGER_BAR);
    postInit();
    mp_errorDialog = new QErrorMessage(mp_mainWindow);
    setActivationWindow(mp_mainWindow);
    mp_mainWindow->show();
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

void KiwixApp::setSideBar(KiwixApp::SideBarType type)
{
    auto sideDockWidget = mp_mainWindow->getSideDockWidget();
    switch(type) {
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
    CREATE_ACTION_ICON(KiwixServeAction, "share", gt("local-kiwix-server"));
    SET_SHORTCUT(KiwixServeAction, QKeySequence(Qt::CTRL+Qt::Key_I));

    CREATE_ACTION_ICON(RandomArticleAction, "random", gt("random-article"));
    SET_SHORTCUT(RandomArticleAction, QKeySequence(Qt::CTRL+Qt::Key_R));
    connect(mpa_actions[RandomArticleAction], &QAction::triggered,
            this, [=]() { this->openRandomUrl(false); });

    CREATE_ACTION(OpenHomePageAction, gt("home-page"));
    SET_SHORTCUT(OpenHomePageAction, QKeySequence(Qt::ALT + Qt::Key_Home));

    CREATE_ACTION_ICON(HistoryBackAction, "back", gt("back"));
    SET_SHORTCUT(HistoryBackAction, QKeySequence(Qt::ALT + Qt::Key_Left));
    DISABLE_ACTION(HistoryBackAction);

    CREATE_ACTION_ICON(HistoryForwardAction, "forward", gt("forward"));
    SET_SHORTCUT(HistoryForwardAction, QKeySequence(Qt::ALT + Qt::Key_Right));
    DISABLE_ACTION(HistoryForwardAction);

    CREATE_ACTION_ICON(PrintAction, "print", gt("print"));
    SET_SHORTCUT(PrintAction, QKeySequence::Print);
    connect(mpa_actions[PrintAction], &QAction::triggered,
            this, &KiwixApp::printPage);

    CREATE_ACTION(NewTabAction, gt("new-tab"));
    SET_SHORTCUT(NewTabAction, QKeySequence::AddTab);

    CREATE_ACTION_ICON(CloseTabAction, "close", gt("close-tab"));
    SET_SHORTCUT(CloseTabAction, QKeySequence::Close);
    mpa_actions[CloseTabAction]->setIconVisibleInMenu(false);

    CREATE_ACTION(ReopenClosedTabAction, gt("reopen-closed-tab"));
    SET_SHORTCUT(ReopenClosedTabAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_T));
    HIDE_ACTION(ReopenClosedTabAction);

    CREATE_ACTION(BrowseLibraryAction, gt("browse-library"));
    SET_SHORTCUT(BrowseLibraryAction, QKeySequence(Qt::CTRL+Qt::Key_E));
    HIDE_ACTION(BrowseLibraryAction);

    CREATE_ACTION_ICON(OpenFileAction, "open-file", gt("open-file"));
    SET_SHORTCUT(OpenFileAction, QKeySequence::Open);
    connect(mpa_actions[OpenFileAction], &QAction::triggered,
            this, [=]() { openZimFile(); });

    CREATE_ACTION(OpenRecentAction, gt("open-recent"));
    HIDE_ACTION(OpenRecentAction);

    /* TODO See https://github.com/kiwix/kiwix-desktop/issues/77
    CREATE_ACTION(SavePageAsAction, tr("Save page as ..."));
    SET_SHORTCUT(SavePageAsAction, QKeySequence::SaveAs);
    HIDE_ACTION(SavePageAsAction);
    */

    CREATE_ACTION(SearchArticleAction, gt("search-article"));
    SET_SHORTCUT(SearchArticleAction, QKeySequence(Qt::CTRL+Qt::Key_L));
    HIDE_ACTION(SearchArticleAction);

    CREATE_ACTION(SearchLibraryAction, gt("search-in-library"));
    SET_SHORTCUT(SearchLibraryAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_R));
    HIDE_ACTION(SearchLibraryAction);

    CREATE_ACTION(FindInPageAction, gt("find-in-page"));
    mpa_actions[FindInPageAction]->setShortcuts({QKeySequence::Find, Qt::Key_F3});
    connect(mpa_actions[FindInPageAction], &QAction::triggered,
            this, [=]() { mp_tabWidget->openFindInPageBar(); });

    CREATE_ACTION_ICON(ToggleFullscreenAction, "full-screen-enter", gt("set-fullscreen"));
    SET_SHORTCUT(ToggleFullscreenAction, QKeySequence::FullScreen);
    connect(mpa_actions[ToggleFullscreenAction], &QAction::toggled,
            this, [=](bool checked) {
        auto action = mpa_actions[ToggleFullscreenAction];
        action->setIcon(
            QIcon(checked ? ":/icons/full-screen-exit.svg" : ":/icons/full-screen-enter.svg"));
        action->setText(checked ? gt("quit-fullscreen") : gt("set-fullscreen"));
    });
    mpa_actions[ToggleFullscreenAction]->setCheckable(true);

    CREATE_ACTION(ToggleTOCAction, gt("table-of-content"));
    SET_SHORTCUT(ToggleTOCAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_1));
    HIDE_ACTION(ToggleTOCAction);

    CREATE_ACTION_ICON(ToggleReadingListAction, "reading-list" ,gt("reading-list"));
    SET_SHORTCUT(ToggleReadingListAction, QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_2));
    connect(mpa_actions[ToggleReadingListAction], &QAction::triggered,
            this, [=]() { toggleSideBar(READINGLIST_BAR); });
    connect(this, &KiwixApp::currentSideTypeChanged,
            this, [=](SideBarType type) {
        mpa_actions[ToggleReadingListAction]->setIcon(
            QIcon((type == READINGLIST_BAR) ? ":/icons/reading-list-active.svg" : ":/icons/reading-list.svg"));
    });

    CREATE_ACTION(ZoomInAction, gt("zoom-in"));
    SET_SHORTCUT(ZoomInAction, QKeySequence::ZoomIn);

    CREATE_ACTION(ZoomOutAction, gt("zoom-out"));
    SET_SHORTCUT(ZoomOutAction, QKeySequence::ZoomOut);

    CREATE_ACTION(ZoomResetAction, gt("zoom-reset"));
    SET_SHORTCUT(ZoomResetAction, QKeySequence(Qt::CTRL+Qt::Key_0));

    CREATE_ACTION(HelpAction, gt("help"));
    SET_SHORTCUT(HelpAction, QKeySequence::HelpContents);
    HIDE_ACTION(HelpAction);

    CREATE_ACTION(FeedbackAction, gt("feedback"));
    HIDE_ACTION(FeedbackAction);

    CREATE_ACTION(ReportBugAction, gt("report-a-bug"));
    HIDE_ACTION(ReportBugAction);

    CREATE_ACTION(RequestFeatureAction, gt("request-a-feature"));
    HIDE_ACTION(RequestFeatureAction);

    CREATE_ACTION(AboutAction, gt("about-kiwix"));

    CREATE_ACTION_ICON(SettingAction, "settings", gt("settings"));
    SET_SHORTCUT(SettingAction, QKeySequence(Qt::Key_F12));

    CREATE_ACTION_ICON(DonateAction, "donate", gt("donate-to-support-kiwix"));
    SET_SHORTCUT(DonateAction, QKeySequence(Qt::CTRL+Qt::Key_D));

    CREATE_ACTION_ICON(ExitAction, "exit", gt("exit"));
    SET_SHORTCUT(ExitAction, QKeySequence::Quit);
}

void KiwixApp::postInit() {
    connect(mp_tabWidget, &TabBar::webActionEnabledChanged,
            mp_mainWindow->getTopWidget(), &TopWidget::handleWebActionEnabledChanged);
    connect(mp_tabWidget, &TabBar::currentTitleChanged, this,
            [=](const QString& title) { emit currentTitleChanged(title); });
    connect(mp_tabWidget, &TabBar::libraryPageDisplayed, this, &KiwixApp::disableItemsOnLibraryPage);
    emit(m_library.booksChanged());
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
