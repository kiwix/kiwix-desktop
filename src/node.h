#ifndef NODE_H
#define NODE_H

#include <QVariant>

class Node
{
public:
    virtual ~Node() = default;
    virtual Node *parentItem() = 0;
    virtual int childCount() const = 0;
    virtual int columnCount() const = 0;
    virtual QVariant data(int column) = 0;
    virtual int row() const = 0;
    virtual QString getBookId() const = 0;
};


#endif // NODE_H
