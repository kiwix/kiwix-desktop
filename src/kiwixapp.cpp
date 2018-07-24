#include "kiwixapp.h"
#include "zim/error.h"

#include <QFontDatabase>
#include <QStyleFactory>
#include <QFile>

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv)
{
    auto icon = QIcon();
    icon.addFile(":/icons/kiwix/app_icon.svg");
    setWindowIcon(icon);

    setApplicationDisplayName("Kiwix");
    setApplicationName("Kiwix");
    setDesktopFileName("kiwix.desktop");

    setStyle(QStyleFactory::create("Windows"));
    QFile styleFile(":/css/style.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    setStyleSheet(style);


    QString fontName;
    if (platformName() == "windows") {
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuib.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuii.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuil.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuisl.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeui.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/segoeuiz.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguibli.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguibl.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguili.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguisbi.ttf");
        QFontDatabase::addApplicationFont(":/fonts/SegoeUI/seguisb.ttf");
        fontName = "Segoe";
    } else {
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Regular.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Light.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-LightItalic.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Medium.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-MediumItalic.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-Bold.ttf");
        QFontDatabase::addApplicationFont(":/fonts/Ubuntu/Ubuntu-BoldItalic.ttf");
        fontName = "Ubuntu";
    }

    auto font = QFont(fontName);
    setFont(font);
    mp_mainWindow = new MainWindow;
    mp_mainWindow->show();
    mp_tabWidget = mp_mainWindow->getTabWidget();

    mp_errorDialog = new QErrorMessage(mp_mainWindow);
}

KiwixApp::~KiwixApp()
{
    delete mp_errorDialog;
    delete mp_mainWindow;
}

KiwixApp *KiwixApp::instance()
{
    return static_cast<KiwixApp*>(QApplication::instance());
}

void KiwixApp::openZimFile(const QString &zimfile)
{
    QString zimId;
    try {
        zimId = m_library.openBook(zimfile);
    } catch (const std::exception& e) {
        showMessage("Cannot open " + zimfile + ": \n" + e.what());
        return;
    }
    openUrl(QUrl("zim://"+zimId+"/"));
}

void KiwixApp::openUrl(const QUrl &url, bool newTab) {
    mp_tabWidget->openUrl(url, newTab);
}

void KiwixApp::showMessage(const QString &message)
{
    mp_errorDialog->showMessage(message);
}
