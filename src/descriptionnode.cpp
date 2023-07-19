#include "descriptionnode.h"
#include "rownode.h"

DescriptionNode::DescriptionNode(QString desc, std::weak_ptr<RowNode> parent)
    : m_desc(desc), m_parentItem(parent)
{}

DescriptionNode::~DescriptionNode()
{}

std::shared_ptr<Node> DescriptionNode::parentItem()
{
    std::shared_ptr<Node> temp = m_parentItem.lock();
    if (!temp)
        return nullptr;
    return temp;
}

QString DescriptionNode::getBookId() const
{
    std::shared_ptr<RowNode> temp = m_parentItem.lock();
    if (!temp)
        return QString();
    return temp->getBookId();
}

int DescriptionNode::childCount() const
{
    return 0;
}

int DescriptionNode::columnCount() const
{
    return 1;
}

QVariant DescriptionNode::data(int column)
{
    if (column == 1)
        return m_desc;
    return QVariant();
}

int DescriptionNode::row() const
{
    std::shared_ptr<RowNode> temp = m_parentItem.lock();
    if (!temp)
        return 0;
    return temp->row();
}
