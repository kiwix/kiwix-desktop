#include "kiwixapp.h"
#include "zim/error.h"

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv),
      reader(nullptr)
{
    mainWindow = new MainWindow;
    setApplicationName("kiwix-desktop");
    mainWindow->show();
}

KiwixApp::~KiwixApp()
{
    delete mainWindow;
    if (reader)
        delete reader;
}


void KiwixApp::openZimFile(const QString &zimfile)
{
    if (reader)
        delete reader;
    const std::string zimfile_ = zimfile.toLocal8Bit().constData();
    std::cout << "Opening " << zimfile_ << std::endl;
    try {
        reader = new kiwix::Reader(zimfile_);
    } catch (const zim::ZimFileFormatError& e) {
        std::cout << "Cannot open " << zimfile_ << std::endl;
        std::cout << e.what() << std::endl;
        reader = nullptr;
    } catch (const std::exception& e) {
        std::cout << "oup" << e.what() << std::endl;
        reader = nullptr;
    }
}

kiwix::Reader* KiwixApp::getReader()
{
    return reader;
}
