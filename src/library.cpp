#include "library.h"


#include <kiwix/manager.h>

#include <QtDebug>

class LibraryManipulator: public kiwix::LibraryManipulator {
  public:
    LibraryManipulator(Library* p_library)
        : mp_library(p_library) {}
    bool addBookToLibrary(kiwix::Book book) {
        auto ret = mp_library->m_library.addBook(book);
        emit(mp_library->booksChanged());
        return ret;
    }
    Library* mp_library;
};

Library::Library()
{
    auto manipulator = LibraryManipulator(this);
    auto manager = kiwix::Manager(&manipulator);
    qInfo() << QString::fromStdString(getDataDirectory());
    manager.readFile(appendToDirectory(getDataDirectory(),"library.xml"), false);
    qInfo() << getBookIds().length();
    emit(booksChanged());
}

QString Library::openBookFromPath(const QString &zimPath)
{
    for(auto it=m_readersMap.begin();
        it != m_readersMap.end();
        it++)
    {
        if(QString::fromStdString(it.value()->getZimFilePath()) == zimPath)
            return it.key();
    }
    qInfo() << "Opening" << zimPath;
    auto zimPath_ = zimPath.toStdString();
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(zimPath_));
    auto _id(reader->getId());
    auto id = QString::fromStdString(_id + ".zim");
    kiwix::Book b;
    b.update(*reader);
    m_library.addBook(b);
    m_readersMap[id] = reader;
    m_library.writeToFile(appendToDirectory(getDataDirectory(),"library.xml"));
    emit(booksChanged());
    return id;
}

QString Library::openBookById(const QString& _id)
{
    auto& b = m_library.getBookById(_id.toStdString());
    auto reader = std::shared_ptr<kiwix::Reader>(new kiwix::Reader(b.getPath()));
    auto id = _id + ".zim";
    m_readersMap[id] = reader;
    return id;
}

std::shared_ptr<kiwix::Reader> Library::getReader(const QString &zimId)
{
    auto it = m_readersMap.find(zimId);
    if (it != m_readersMap.end())
        return it.value();
    return nullptr;
}

QStringList Library::getBookIds()
{
    QStringList list;
    for(auto& id: m_library.getBooksIds()) {
        list.append(QString::fromStdString(id));
    }
    return list;
}

#define ADD_V(KEY, METH) {if(key==KEY) values.append(QString::fromStdString((b.METH())));}
QStringList Library::getBookInfos(QString id, const QStringList &keys)
{
    QStringList values;
    if (id.endsWith(".zim")) {
        id.resize(id.size()-4);
    }
    auto& b = m_library.getBookById(id.toStdString());
    for(auto& key: keys){
        ADD_V("id", getId);
        ADD_V("path", getPath);
        ADD_V("indexPath", getIndexPath);
        ADD_V("title", getTitle);
        ADD_V("description", getDescription);
        ADD_V("language", getLanguage);
        ADD_V("creator", getCreator);
        ADD_V("publisher", getPublisher);
        ADD_V("date", getDate);
        ADD_V("url", getUrl);
        ADD_V("name", getName);
        ADD_V("tags", getTags);
        ADD_V("origId", getOrigId);
        ADD_V("faviconMimeType", getFaviconMimeType);
        ADD_V("downloadId", getDownloadId);
        if (key == "favicon") {
            auto s = b.getFavicon();
            values.append(QByteArray::fromStdString(s).toBase64());
        }
    }
    return values;
}
