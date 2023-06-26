#include "node.h"

Node::Node(const QList<QVariant> &data, Node *parent, QString bookId, bool isAdditional)
    : m_itemData(data), m_parentItem(parent),  m_isAdditonal(isAdditional), m_bookId(bookId)
{
    m_downloadInfo = {0, "", "", false};
}

Node::~Node()
{
    qDeleteAll(m_childItems);
}

void Node::appendChild(Node *item)
{
    m_childItems.append(item);
}

Node *Node::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int Node::childCount() const
{
    return m_childItems.count();
}

int Node::columnCount() const
{
    return m_itemData.count();
}

QVariant Node::data(int column) const
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

Node *Node::parentItem()
{
    return m_parentItem;
}

int Node::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<Node*>(this));

    return 0;
}
