#include <QtGlobal>

#include "kiwixapp.h"

#include <QCommandLineParser>
#include <iostream>
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  #include <QWebEngineUrlScheme>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QWebEngineUrlScheme scheme("zim");
    QWebEngineUrlScheme::registerScheme(scheme);
#endif
    KiwixApp a(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument("zimfile", "The zim file");

    parser.process(a);
    QString zimfile;
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() >= 1){
        zimfile = parser.positionalArguments().at(0);
        a.openZimFile(zimfile);
    }
    return a.exec();
}
