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
        SavePageAsAction,
        SearchArticleAction,
        SearchLibraryAction,
        FindInPageAction,
        ToggleFullscreenAction,
        ToggleTOCAction,
        ToggleReadingListAction,
        ToggleAddBookmarkAction,
        ZoomInAction,
        ZoomOutAction,
        ZoomResetAction,
        NextTabAction,
        PreviousTabAction,
        HistoryBackAction,
        HistoryForwardAction,
        HelpAction,
        FeedbackAction,
        ReportBugAction,
        RequestFeatureAction,
        AboutAction,
        SettingAction,
        DonateAction,
        ExitAction,
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
    kiwix::Downloader* getDownloader() { return mp_downloader; }
    TabBar* getTabWidget() { return getMainWindow()->getTabBar(); }
    QAction* getAction(Actions action);
    QString getLibraryDirectory() { return m_libraryDirectory; };
    kiwix::Server* getLocalServer() { return &m_server; }
    SettingsManager* getSettingsManager() { return &m_settingsManager; };
    QString getText(const QString &key) { return m_translation.getText(key); };
    void setMonitorDir(const QString &dir);
    bool isCurrentArticleBookmarked();
    QString parseStyleFromFile(QString filePath);
    void saveListOfOpenTabs();

public slots:
    void newTab();
    void openZimFile(const QString& zimfile="");
    void openUrl(const QString& url, bool newTab=true);
    void openUrl(const QUrl& url, bool newTab=true);
    void printPage();
    void handleItemsState(TabType);
    void updateNameMapper();
    void printVersions(std::ostream& out = std::cout);

protected:
    void createActions();
    void postInit();

private:
    QTranslator m_qtTranslator, m_appTranslator;
    SettingsManager m_settingsManager;
    KProfile m_profile;
    QString m_libraryDirectory;
    Library m_library;
    kiwix::Downloader* mp_downloader;
    ContentManager* mp_manager;
    MainWindow* mp_mainWindow;
    QErrorMessage* mp_errorDialog;
    std::shared_ptr<kiwix::UpdatableNameMapper> mp_nameMapper;
    kiwix::Server m_server;
    Translation m_translation;
    QFileSystemWatcher m_watcher;
    QSettings* mp_session;

    QAction*     mpa_actions[MAX_ACTION];

    QString findLibraryDirectory();
    void restoreTabs();
    void loadAndInstallTranslations(QTranslator& translator, const QString& filename, const QString& directory);
};

QString gt(const QString &key);
#define _STR(...) # __VA_ARGS__
#define STR(X) _STR(X)
static QString version = STR(VERSION);
#endif // KIWIXAPP_H
