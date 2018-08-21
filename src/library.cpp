#include "library.h"

#include <QtWidgets>
#include <QtDebug>

Library::Library()
{

}

QString Library::openBook(const QString &zimPath)
{
    for(auto it=m_readersMap.begin();
        it != m_readersMap.end();
        it++)
    {
        if(QString::fromStdString(it->second->getZimFilePath()) == zimPath)
            return it->first;
    }
    qInfo() << tr("Opening") << zimPath;
    auto zimPath_ = zimPath.toStdString();
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(zimPath_));
    auto id = QString::fromStdString(reader->getId() + ".zim");
    m_readersMap[id] = reader;
    return id;
}

std::shared_ptr<kiwix::Reader> Library::getReader(const QString &zimId)
{
    auto it = m_readersMap.find(zimId);
    if (it != m_readersMap.end())
        return it->second;
    return nullptr;
}
