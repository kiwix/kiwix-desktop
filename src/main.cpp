#include <QtGlobal>

#include "kiwixapp.h"

#include <QCommandLineParser>
#include <iostream>
#include <sstream>
#ifdef Q_OS_WIN
    #include <kiwix/tools.h>
    #include <windows.h>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  #include <QWebEngineUrlScheme>
#endif
int main(int argc, char *argv[])
{
// Small hack to make QtWebEngine works with AppImage.
// See https://github.com/probonopd/linuxdeployqt/issues/554
    if (qEnvironmentVariableIsSet("APPIMAGE")) {
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu --no-sandbox");
        QApplication::setAttribute(Qt::AA_UseOpenGLES);
    }
// End of hack ^^^

#ifdef Q_OS_WIN
    std::string driveLetter = kiwix::getExecutablePath().substr(0, 3);
    UINT driveType = GetDriveTypeA(driveLetter.c_str());
    if(driveType == DRIVE_REMOTE) {
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray("--no-sandbox"));
        qInfo() << "Disabled sandbox";
    }
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // High DPI Scaling is enabled by default in Qt6. This attribute no longer exists in 6.0 and later
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
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
