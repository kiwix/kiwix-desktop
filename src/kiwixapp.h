#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "contentmanager.h"
#include "tabbar.h"
#include "mainwindow.h"
#include "kiwix/downloader.h"
#include <kiwix/kiwixserve.h>
#include "kprofile.h"
#include "settingsmanager.h"
#include "translation.h"

#include <QtSingleApplication>
#include <QApplication>
#include <QErrorMessage>
#include <QMessageBox>
#include <QTranslator>
#include <kiwix/name_mapper.h>

#include <mutex>
#include <iostream>

typedef TabBar::TabType TabType;

class KiwixApp : public QtSingleApplication
{
    Q_OBJECT

public:
    enum Actions {
        KiwixServeAction,
        RandomArticleAction,
        OpenHomePageAction,
        PrintAction,
        NewTabAction,
        CloseCurrentTabAction,
        ReopenClosedTabAction,
        BrowseLibraryAction,
        OpenFileAction,
        OpenRecentAction,
        OpenMultiZimAction,
        SavePageAsAction,
        SearchArticleAction,
        SearchLibraryAction,
        FindInPageAction,
        ToggleFullscreenAction,
        ToggleTOCAction,
        ToggleReadingListAction,
        ToggleAddBookmarkAction,
        ToggleTTSLanguageAction,
        ToggleTTSVoiceAction,
        IncreaseTTSSpeedAction,
        DecreaseTTSSpeedAction,
        ZoomInAction,
        ZoomOutAction,
        ZoomResetAction,
        NextTabAction,
        PreviousTabAction,
        HistoryBackAction,
        HistoryForwardAction,
        ReadTextAction,
        ReadArticleAction,
        ReadStopAction,
        HelpAction,
        FeedbackAction,
        ReportBugAction,
        RequestFeatureAction,
        AboutAction,
        SettingAction,
        DonateAction,
        ExitAction,
        ExportReadingListAction,
        ImportReadingListAction,
        ScrollPreviousTabAction,
        ScrollNextTabAction,
        MAX_ACTION
    };

    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();
    static KiwixApp* instance();
    void init();

    void openRandomUrl(bool newTab=true);

    void showMessage(const QString& message, const QString& title, const enum QMessageBox::Icon& icon);

    KProfile* getProfile() { return &m_profile; }
    Library* getLibrary() { return &m_library; }
    MainWindow* getMainWindow() { return mp_mainWindow; }
    ContentManager* getContentManager() { return mp_manager; }
    TabBar* getTabWidget() { return getMainWindow()->getTabBar(); }
    SearchBar& getSearchBar() { return getMainWindow()->getTopWidget()->getSearchBar(); }
    QAction* getAction(Actions action);
    QString getLibraryDirectory() { return m_libraryDirectory; };
    kiwix::Server* getLocalServer() { return &m_server; }
    SettingsManager* getSettingsManager() { return &m_settingsManager; };
    QString getText(const QString &key) { return m_translation.getText(key); };
    void setMonitorDir(const QString &dir);
    bool isCurrentArticleBookmarked();
    void saveListOfOpenTabs();
    void saveWindowState();
    void saveVoiceName(const QString& langName, const QString& voiceName);
    void saveTtsSpeed(const QString& langName, double speed);
    void restoreWindowState();
    void saveCurrentTabIndex();
    void savePrevSaveDir(const QString& prevSaveDir);
    QString getSavedVoiceName(const QString& langName) const;
    double getSavedTtsSpeed(const QString& langName) const;
    QString getPrevSaveDir() const;
    void restoreTabs();
    void setupDirectoryMonitoring();

    /**
     * @brief Returns true when the effective UI theme is dark.
     *
     * "Dark" wins unconditionally when the user selected Dark.
     * "System" delegates to QGuiApplication::styleHints() on Qt >= 6.5;
     * it falls back to false (light) on older versions.
     * "Light" always returns false.
     */
    bool isDarkTheme() const;

    /**
     * @brief Returns the Qt resource path for the global stylesheet
     *        that matches the current effective theme.
     */
    QString getThemeCssPath() const;

public slots:
    void newTab();
    void openZimFile(const QString& zimfile="");
    void openUrl(const QString& url, bool newTab=true);
    void openUrl(const QUrl& url, bool newTab=true);
    void printPage();
    void handleItemsState(TabType);
    void updateNameMapper();
    void printVersions(std::ostream& out = std::cout);
    void applyTheme(SettingsManager::Theme theme);

protected:
    void createActions();
    void postInit();

private:
    QTranslator m_qtTranslator, m_appTranslator;
    SettingsManager m_settingsManager;
    KProfile m_profile;
    QString m_libraryDirectory;
    Library m_library;
    ContentManager* mp_manager;
    MainWindow* mp_mainWindow;
    QErrorMessage* mp_errorDialog;
    std::shared_ptr<kiwix::UpdatableNameMapper> mp_nameMapper;
    kiwix::Server m_server;
    Translation m_translation;
    QSettings* mp_session;

    QAction*     mpa_actions[MAX_ACTION];

    QString findLibraryDirectory();
    void loadAndInstallTranslations(QTranslator& translator, const QString& filename, const QString& directory);
};

QString getFileContent(QString filePath);
QString gt(const QString &key);
/**
 * @brief Returns the CSS content for the correct theme variant.
 *
 * @param baseCssPath     Qt resource path for the default/light stylesheet.
 * @param customDarkPath  Optional explicit Qt resource path for the dark stylesheet.
 *                        If omitted, "_dark.css" is automatically used if available.
 * @return Content of whichever file matches the effective current theme.
 */
QString themeAwareCss(const QString &baseCssPath, const QString &customDarkPath = QString());
#define _STR(...) # __VA_ARGS__
#define STR(X) _STR(X)
static QString version = STR(VERSION);
#endif // KIWIXAPP_H
