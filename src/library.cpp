#include "library.h"
#include <QtDebug>

Library::Library()
{

}

QString Library::openBook(const QString &zimPath)
{
    for(auto it=readers_map.begin();
        it != readers_map.end();
        it++)
    {
        if(QString::fromStdString(it->second->getZimFilePath()) == zimPath)
            return it->first;
    }
    qInfo() << "Opening" << zimPath;
    const std::string zimPath_ = zimPath.toLocal8Bit().constData();
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(zimPath_));
    auto id = QString::fromStdString(reader->getId() + ".zim");
    readers_map[id] = reader;
    return id;
}

std::shared_ptr<kiwix::Reader> Library::getReader(const QString &zimId)
{
    auto it = readers_map.find(zimId);
    if (it != readers_map.end())
        return it->second;
    return nullptr;
}
