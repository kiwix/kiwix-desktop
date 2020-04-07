#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "contentmanager.h"
#include "mainwindow.h"
#include "kiwix/downloader.h"
#include <kiwix/kiwixserve.h>
#include "tabbar.h"
#include "kprofile.h"
#include "urlschemehandler.h"
#include "settingsmanager.h"
#include "translation.h"

#include <QApplication>
#include <QErrorMessage>
#include <QTranslator>


class KiwixApp : public QApplication
{
    Q_OBJECT
    Q_PROPERTY(SideBarType currentSideType MEMBER m_currentSideType NOTIFY currentSideTypeChanged)

public:
    enum Actions {
        KiwixServeAction,
        RandomArticleAction,
        OpenHomePageAction,
        PrintAction,
        NewTabAction,
        CloseTabAction,
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
        ZoomInAction,
        ZoomOutAction,
        ZoomResetAction,
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
    enum SideBarType {
        CONTENTMANAGER_BAR,
        READINGLIST_BAR,
        NONE
    };

    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();
    static KiwixApp* instance();

    void openRandomUrl(bool newTab=true);

    void showMessage(const QString& message);

    KProfile* getProfile() { return &m_profile; }
    Library* getLibrary() { return &m_library; }
    MainWindow* getMainWindow() { return mp_mainWindow; }
    kiwix::Downloader* getDownloader() { return mp_downloader; }
    TabBar* getTabWidget() { return mp_tabWidget; }
    QAction* getAction(Actions action);
    QString getLibraryDirectory() { return m_libraryDirectory; };
    kiwix::KiwixServe* getLocalServer() { return mp_server; }
    SettingsManager* getSettingsManager() { return &m_settingsManager; };
    SideBarType getSideType() { return m_currentSideType; }
    QString getText(const QString &key) { return m_translation.getText(key); };

    bool isCurrentArticleBookmarked();

signals:
    void currentTitleChanged(const QString& title);
    void currentSideTypeChanged(SideBarType type);

public slots:
    void openZimFile(const QString& zimfile="");
    void openUrl(const QString& url, bool newTab=true);
    void openUrl(const QUrl& url, bool newTab=true);
    void setSideBar(SideBarType type);
    void toggleSideBar(KiwixApp::SideBarType type);
    void printPage();
    void disableItemsOnLibraryPage(bool displayed);

protected:
    void createAction();
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
    TabBar* mp_tabWidget;
    SideBarType m_currentSideType;
    QErrorMessage* mp_errorDialog;
    kiwix::KiwixServe* mp_server;
    Translation m_translation;

    QAction*     mpa_actions[MAX_ACTION];

    QString findLibraryDirectory();
};

QString gt(const QString &key);

#endif // KIWIXAPP_H
