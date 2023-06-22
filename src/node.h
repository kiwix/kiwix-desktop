#ifndef NODE_H
#define NODE_H

#include <QVariant>
#include <QList>

class Node
{
public:
    explicit Node(const QList<QVariant> &data, Node *parentItem = nullptr, bool isAdditional = false);
    ~Node();
    void appendChild(Node *child);
    Node *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    Node *parentItem();
    bool isAdditonal() const { return m_isAdditonal; }

private:
    QList<QVariant> m_itemData;
    Node *m_parentItem;
    QList<Node *> m_childItems;
    bool m_isAdditonal;
};


#endif // NODE_H
