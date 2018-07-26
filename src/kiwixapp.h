#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "mainwindow.h"
#include "tabwidget.h"
#include "urlschemehandler.h"
#include "requestinterceptor.h"

#include <QApplication>
#include <QErrorMessage>


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
        FindNextAction,
        FindPreviousAction,
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

    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();
    static KiwixApp* instance();

    void openUrl(const QUrl& url, bool newTab=true);

    void showMessage(const QString& message);

    UrlSchemeHandler* getSchemeHandler() { return &m_schemeHandler; }
    RequestInterceptor* getRequestInterceptor() { return &m_requestInterceptor; }
    Library* getLibrary() { return &m_library; }
    MainWindow* getMainWindow() { return mp_mainWindow; }
    TabWidget* getTabWidget() { return mp_tabWidget; }
    QAction* getAction(Actions action);

public slots:
    void openZimFile(const QString& zimfile="");
    void printPage();

protected:
    void createAction();

private:
    Library m_library;
    MainWindow* mp_mainWindow;
    TabWidget* mp_tabWidget;
    QErrorMessage* mp_errorDialog;

    UrlSchemeHandler m_schemeHandler;
    RequestInterceptor m_requestInterceptor;
    QAction*     mpa_actions[MAX_ACTION];
};

#endif // KIWIXAPP_H
