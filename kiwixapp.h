#ifndef KIWIXAPP_H
#define KIWIXAPP_H

#include "kiwixschemehandler.h"
#include "kiwixrequestinterceptor.h"

#include <QApplication>
#include <kiwix/reader.h>


class KiwixApp : public QApplication
{
public:
    KiwixApp(int& argc, char *argv[]);
    virtual ~KiwixApp();

    void openZimFile(const QString& zimfile);
    kiwix::Reader* getReader();


private:
    kiwix::Reader* reader;
};

#endif // KIWIXAPP_H
