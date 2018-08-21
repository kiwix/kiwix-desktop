#include "kiwixapp.h"

#include <QCommandLineParser>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    KiwixApp a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                             QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("kiwix-desktop_" + QLocale::system().name());
    a.installTranslator(&myappTranslator);

    QCommandLineParser parser;
    parser.addPositionalArgument("zimfile", "The zim file");

    parser.process(a);
    QString zimfile;
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() >= 1){
        zimfile = parser.positionalArguments().at(0);
    }
    a.openZimFile(zimfile);
    return a.exec();
}
