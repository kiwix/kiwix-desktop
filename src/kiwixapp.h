#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "mainwindow.h"
#include "ktabwidget.h"
#include "kiwixschemehandler.h"
#include "kiwixrequestinterceptor.h"

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

    KiwixSchemeHandler* getSchemeHandler() { return &schemeHandler; }
    KiwixRequestInterceptor* getRequestInterceptor() { return &requestIntercetor; }
    Library* getLibrary() { return &library; }
    MainWindow* getMainWindow() { return mainWindow; }
    KTabWidget* getTabWidget() { return tabWidget; }


private:
    Library library;
    MainWindow* mainWindow;
    KTabWidget* tabWidget;
    QErrorMessage* errorDialog;

    KiwixSchemeHandler schemeHandler;
    KiwixRequestInterceptor requestIntercetor;
};

#endif // KIWIXAPP_H
