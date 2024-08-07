#include "rownode.h"
#include <QVariant>
#include "kiwixapp.h"
#include "descriptionnode.h"

////////////////////////////////////////////////////////////////////////////////
// RowNode
////////////////////////////////////////////////////////////////////////////////

RowNode::RowNode(QList<QVariant> itemData, QString bookId, std::weak_ptr<RowNode> parent)
    : m_itemData(itemData), m_parentItem(parent), m_bookId(bookId)
{
}

RowNode::~RowNode()
{}

void RowNode::appendChild(std::shared_ptr<Node> item)
{
    m_childItems.append(item);
}

std::shared_ptr<Node> RowNode::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int RowNode::childCount() const
{
    return m_childItems.count();
}

int RowNode::columnCount() const
{
    return 6;
}

std::shared_ptr<Node> RowNode::parentItem()
{
    std::shared_ptr<Node> temp = m_parentItem.lock();
    if (!temp)
        return nullptr;
    return temp;
}

QVariant RowNode::data(int column)
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

int RowNode::row() const
{
    try {
        std::shared_ptr<RowNode> temp = m_parentItem.lock();
        if (temp) {
            auto nodePtr = std::const_pointer_cast<Node>(shared_from_this());
            return temp->m_childItems.indexOf(nodePtr);
        }
    } catch(...) {
        return 0;
    }

    return 0;
}

bool RowNode::isChild(Node *candidate)
{
    if (!candidate)
        return false;
    for (auto item : m_childItems) {
        if (candidate == item.get())
            return true;
    }
    return false;
}

void RowNode::setDownloadState(std::shared_ptr<DownloadState> ds)
{
    m_downloadState = ds;
}
