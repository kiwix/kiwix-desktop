#ifndef LIBRARY_H
#define LIBRARY_H

#include <kiwix/manager.h>
#include <map>
#include <qstring.h>

class Library : public kiwix::Manager
{
public:
    Library();
    QString openBook(const QString& zimPath);
    std::shared_ptr<kiwix::Reader> getReader(const QString& zimId);
private:
    std::map<QString, std::shared_ptr<kiwix::Reader>> readers_map;
};

#endif // LIBRARY_H
