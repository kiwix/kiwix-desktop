#include "kiwixapp.h"
#include "mainwindow.h"

#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    KiwixApp a(argc, argv);
    KiwixApp::setApplicationName("kiwix-desktop");


    QCommandLineParser parser;
    parser.addPositionalArgument("zimfile", "The zim file");

    parser.process(a);
    const QString zimfile = parser.positionalArguments().at(0);

    a.openZimFile(zimfile);

    MainWindow w;
    w.show();

    return a.exec();
}
