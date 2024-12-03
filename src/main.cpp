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

#ifdef Q_OS_WIN
bool wasAppStartedFromARemoteDrive()
{
    const std::string exePath = kiwix::getExecutablePath();

    return GetDriveTypeA(exePath.substr(0, 3).c_str()) == DRIVE_REMOTE;
}
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
    const bool appWasStartedFromARemoteDrive = wasAppStartedFromARemoteDrive();
    if ( appWasStartedFromARemoteDrive ) {
	// This has to be done early before the qtwebengine dll is loaded.
	// We notify the user about the implications and ask for confirmation
	// a little later (see closer to the bottom of this function).
	qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray("--no-sandbox"));
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
#ifdef Q_OS_WIN
    if ( appWasStartedFromARemoteDrive ) {
        int result = QMessageBox::question(nullptr, gt("about-kiwix-desktop-title"), gt("disable-sandbox"), QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) return 0;
    }
#endif
    for (QString zimfile : positionalArguments) {
        a.openZimFile(zimfile);
    }
    return a.exec();
}
