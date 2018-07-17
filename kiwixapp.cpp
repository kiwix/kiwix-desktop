#include "kiwixapp.h"
#include "zim/error.h"

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv)
{
    auto icon = QIcon();
    icon.addFile(":/icons/kiwix/app_icon.svg");
    setWindowIcon(icon);

    setApplicationDisplayName("Kiwix");
    setApplicationName("Kiwix");
    setDesktopFileName("kiwix.desktop");

    mainWindow = new MainWindow;
    mainWindow->show();

    errorDialog = new QErrorMessage(mainWindow);
}

KiwixApp::~KiwixApp()
{
    delete errorDialog;
    delete mainWindow;
}


void KiwixApp::openZimFile(const QString &zimfile)
{
    try {
        auto zimId = library.openBook(zimfile);
        mainWindow->displayReader(library.getReader(zimId));
    } catch (const std::exception& e) {
        showMessage("Cannot open " + zimfile + ": \n" + e.what());
    }
}

void KiwixApp::showMessage(const QString &message)
{
    errorDialog->showMessage(message);
}
