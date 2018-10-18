#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "contentmanager.h"
#include "mainwindow.h"
#include "kiwix/downloader.h"
#include "tabbar.h"
#include "tocsidebar.h"
#include "urlschemehandler.h"
#include "requestinterceptor.h"

#include <QApplication>
#include <QErrorMessage>
#include <QTranslator>


class KiwixApp : public QApplication
{
    Q_OBJECT
public:
    enum Actions {
        KiwixServeAction,
        RandomArticleAction,
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
        SEARCH_BAR,
        NONE
    };

    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();
    static KiwixApp* instance();

    void openRandomUrl(bool newTab=true);

    void showMessage(const QString& message);

    UrlSchemeHandler* getSchemeHandler() { return &m_schemeHandler; }
    RequestInterceptor* getRequestInterceptor() { return &m_requestInterceptor; }
    Library* getLibrary() { return &m_library; }
    MainWindow* getMainWindow() { return mp_mainWindow; }
    kiwix::Downloader* getDownloader() { return &m_downloader; }
    TabBar* getTabWidget() { return mp_tabWidget; }
    QAction* getAction(Actions action);

public slots:
    void openZimFile(const QString& zimfile="");
    void openUrl(const QString& url, bool newTab=true);
    void openUrl(const QUrl& url, bool newTab=true);
    void setSideBar(SideBarType type);
    void printPage();

protected:
    void createAction();
    void postInit();

private:
    QTranslator m_qtTranslator, m_appTranslator;
    Library m_library;
    kiwix::Downloader m_downloader;
    ContentManager m_manager;
    MainWindow* mp_mainWindow;
    TabBar* mp_tabWidget;
    QErrorMessage* mp_errorDialog;

    UrlSchemeHandler m_schemeHandler;
    RequestInterceptor m_requestInterceptor;
    QAction*     mpa_actions[MAX_ACTION];
};

#endif // KIWIXAPP_H
