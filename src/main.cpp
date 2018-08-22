#include "kiwixapp.h"

#include <QTranslator>
#include <QLibraryInfo>
#include <QCommandLineParser>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    KiwixApp a(argc, argv);

    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
    QLocale::setDefault(defaultLocale);
    
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                             QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load(":/i18n/kiwix-desktop.qm");
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
