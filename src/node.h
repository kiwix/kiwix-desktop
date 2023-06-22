#ifndef NODE_H
#define NODE_H

#include <QVariant>
#include <QList>

class Node
{
public:
    explicit Node(const QList<QVariant> &data, Node *parentNode = nullptr);
    ~Node();
    void appendChild(Node *child);
    Node *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    Node *parentItem();

private:
    QList<QVariant> m_itemData;
    Node *m_parentItem;
    QList<Node *> m_childItems;
};


#endif // NODE_H
