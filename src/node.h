#ifndef NODE_H
#define NODE_H

#include <QVariant>
#include <QList>
#include "contentmanagermodel.h"
#include <QIcon>

class Node
{
public:
    explicit Node(const QList<QVariant> &data, Node *parentItem = nullptr, QString bookId = "", bool isAdditional = false);
    ~Node();
    void appendChild(Node *child);
    Node *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    Node *parentItem();
    bool isAdditonal() const { return m_isAdditonal; }
    QString getBookId() const { return m_bookId; }
    void setIconData(QByteArray iconData) { m_itemData[0] = iconData; }

private:
    QList<QVariant> m_itemData;
    Node *m_parentItem;
    QList<Node *> m_childItems;
    bool m_isAdditonal;
    QString m_bookId;
};


#endif // NODE_H
