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
    a.setApplicationVersion(version);
    QCommandLineParser parser;
    QString zimfile;
    parser.setApplicationDescription(QStringLiteral("The Kiwix Desktop is a viewer/manager of ZIM files for GNU/Linux and Microsoft Windows OSes."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("zimfile", "The zim file");
    parser.process(a);
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() >= 1) {
        zimfile = parser.positionalArguments().at(0);
    }
    if (a.isRunning()) {
        a.sendMessage(zimfile);
        return 0;
    }
    a.init();
    if (!zimfile.isEmpty()) {
        a.openZimFile(zimfile);
    }
    return a.exec();
}
