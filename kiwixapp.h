#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "library.h"
#include "mainwindow.h"
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

    KiwixSchemeHandler* getSchemeHandler() { return &schemeHandler; }
    KiwixRequestInterceptor* getRequestInterceptor() { return &requestIntercetor; }
    Library* getLibrary() { return &library; }
    MainWindow* getMainWindow() { return mainWindow; }

    void showMessage(const QString& message);
private:
    Library library;
    MainWindow* mainWindow;
    QErrorMessage* errorDialog;

    KiwixSchemeHandler schemeHandler;
    KiwixRequestInterceptor requestIntercetor;
};

#endif // KIWIXAPP_H
