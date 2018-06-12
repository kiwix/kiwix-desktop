#include "kiwixapp.h"

KiwixApp::KiwixApp(int& argc, char *argv[])
    : QApplication(argc, argv),
      reader(nullptr)
{
}

KiwixApp::~KiwixApp()
{
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
    } catch (...) {
        reader = nullptr;
    }
}

kiwix::Reader* KiwixApp::getReader()
{
    return reader;
}
