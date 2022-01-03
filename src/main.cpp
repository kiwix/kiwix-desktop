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
    parser.setApplicationDescription(QStringLiteral("The Kiwix Desktop is a viewer/manager of ZIM files for GNU/Linux and Microsoft Windows OSes."));
    parser.addHelpOption();
    parser.addPositionalArgument("zimfile", "The zim file");

    // Set version string
    std::ostringstream versions;
    a.printVersions(versions);
    a.setApplicationVersion(QString::fromStdString(versions.str()));
    parser.addVersionOption();

    parser.process(a);
    auto positionalArguments = parser.positionalArguments();
    if (a.isRunning()) {
        for (QString zimfile : positionalArguments) {
            a.sendMessage(zimfile);
        }
        return 0;
    }
    a.init();
    for (QString zimfile : positionalArguments) {
        a.openZimFile(zimfile);
    }
    return a.exec();
}
