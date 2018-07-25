#include "kiwixapp.h"

#include <QCommandLineParser>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    KiwixApp a(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument("zimfile", "The zim file");

    parser.process(a);
    QString zimfile;
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() >= 1){
        zimfile = parser.positionalArguments().at(0);
    }
    a.openZimFile(zimfile);
    return a.exec();
}
