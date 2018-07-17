#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "kiwixschemehandler.h"
#include "kiwixrequestinterceptor.h"

#include <QApplication>
#include "library.h"
#include "mainwindow.h"


class KiwixApp : public QApplication
{
public:
    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();

    void openZimFile(const QString& zimfile);

    KiwixSchemeHandler* getSchemeHandler() { return &schemeHandler; }
    KiwixRequestInterceptor* getRequestInterceptor() { return &requestIntercetor; }
    Library* getLibrary() { return &library; }
private:
    Library library;
    MainWindow* mainWindow;

    KiwixSchemeHandler schemeHandler;
    KiwixRequestInterceptor requestIntercetor;
};

#endif // KIWIXAPP_H
