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
public:
    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();
    static KiwixApp* instance();

    void openZimFile(const QString& zimfile);
    void openUrl(const QUrl& url, bool newTab=true);

    void showMessage(const QString& message);

    UrlSchemeHandler* getSchemeHandler() { return &schemeHandler; }
    RequestInterceptor* getRequestInterceptor() { return &requestIntercetor; }
    Library* getLibrary() { return &library; }
    MainWindow* getMainWindow() { return mainWindow; }
    TabWidget* getTabWidget() { return tabWidget; }


private:
    Library library;
    MainWindow* mainWindow;
    TabWidget* tabWidget;
    QErrorMessage* errorDialog;

    UrlSchemeHandler schemeHandler;
    RequestInterceptor requestIntercetor;
};

#endif // KIWIXAPP_H
