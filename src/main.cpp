#include <QtGlobal>

#include "kiwixapp.h"
#include "useragent.h"

#include <QCommandLineParser>
#include <cstring>
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

    // Usual mounting point for Windows Network Drives
    if ( exePath.substr(0, 2) == "\\\\" )
        return true;

    // Support macOS Parallels shared folders.
    // See https://kb.parallels.com/130138
    if ( exePath.substr(0, 7) == "C:\\Mac\\" )
        return true;

    // Last chance to identify as remote drive
    return GetDriveTypeA(exePath.substr(0, 3).c_str()) == DRIVE_REMOTE;
}
#endif

/*
 * Returns true if the given argument is a help or version flag.
 * This allows us to handle --help/--version before instantiating KiwixApp
 * (and the entire Qt application machinery), avoiding QThreadStorage
 * cleanup warnings on exit (issue #1561).
 */
static bool isHelpOrVersionFlag(const char* arg)
{
    return std::strcmp(arg, "-h") == 0
        || std::strcmp(arg, "--help") == 0
        || std::strcmp(arg, "--help-all") == 0
        || std::strcmp(arg, "-?") == 0
        || std::strcmp(arg, "-v") == 0
        || std::strcmp(arg, "--version") == 0;
}

int main(int argc, char *argv[])
{
    // Handle --help and --version before creating KiwixApp to avoid
    // QThreadStorage destruction warnings (issue #1561).
    for (int i = 1; i < argc; ++i) {
        if (isHelpOrVersionFlag(argv[i])) {
            const char* execName = (argc > 0 && argv[0] && argv[0][0] != '\0')
                                 ? argv[0]
                                 : "kiwix-desktop";
            if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--version") == 0) {
                KiwixApp::printVersions();
            } else {
                KiwixApp::printUsage(execName);
            }
            return 0;
        }
    }

#ifdef Q_OS_WIN
    const bool appWasStartedFromARemoteDrive = wasAppStartedFromARemoteDrive();
    if ( appWasStartedFromARemoteDrive ) {
	// This has to be done early before the qtwebengine dll is loaded.
	// We notify the user about the implications and ask for confirmation
	// a little later (see closer to the bottom of this function).
	qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray("--no-sandbox"));
    }
#endif

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // High DPI Scaling is enabled by default in Qt6. This attribute no longer exists in 6.0 and later
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QWebEngineUrlScheme scheme("zim");
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    // Needed so the Fetch API / XHR can load zim:// resources (issue #1476).
    // FetchApiAllowed was introduced in Qt 6.6.
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme
                  | QWebEngineUrlScheme::LocalAccessAllowed
                  | QWebEngineUrlScheme::CorsEnabled
                  | QWebEngineUrlScheme::FetchApiAllowed);
#endif
    QWebEngineUrlScheme::registerScheme(scheme);
#endif

    // Share the desktop User-Agent with libkiwix so that the requests it makes
    // on our behalf (book illustrations, downloads made through a Downloader)
    // are identified as Kiwix Desktop too. This has to happen before the first
    // Downloader is created, because aria2c receives the User-Agent when it is
    // launched (see kiwix/libkiwix#1364).
    shareUserAgentWithLibkiwix();

    KiwixApp a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("The Kiwix Desktop is a viewer/manager of ZIM files for GNU/Linux and Microsoft Windows OSes."));
    parser.addHelpOption();
    parser.addPositionalArgument("zimfile", "The zim file");

    // Set version string
    std::ostringstream versions;
    KiwixApp::printVersions(versions);
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

    /* Restore Tabs before directory monitoring to ensure we know what tabs user had. */
    a.restoreTabs();
    a.setupDirectoryMonitoring();

    for (QString zimfile : positionalArguments) {
        a.openZimFile(zimfile);
    }
    return a.exec();
}
