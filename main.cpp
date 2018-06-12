#include "kiwixapp.h"
#include "mainwindow.h"

#include <QCommandLineParser>
#include <QFileDialog>
#include <iostream>

int main(int argc, char *argv[])
{
    KiwixApp a(argc, argv);
    KiwixApp::setApplicationName("kiwix-desktop");

    QCommandLineParser parser;
    parser.addPositionalArgument("zimfile", "The zim file");

    parser.process(a);
    QString zimfile;
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() < 1){
        zimfile = QFileDialog::getOpenFileName(nullptr,
            "Open Zim",
            QString(),
            "ZimFile (*.zim*)");
    } else {
        zimfile = parser.positionalArguments().at(0);
    }
    std::string f = zimfile.toUtf8().constData();
    std::cout << f << std::endl;

    MainWindow w;
    w.show();
    a.openZimFile(zimfile);

    return a.exec();
}
