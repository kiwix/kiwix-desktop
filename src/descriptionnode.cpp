#include "descriptionnode.h"
#include "rownode.h"

DescriptionNode::DescriptionNode(QString desc, RowNode *parent)
    : m_desc(desc), m_parentItem(parent)
{}

DescriptionNode::~DescriptionNode()
{}

Node* DescriptionNode::parentItem()
{
    return m_parentItem;
}

QString DescriptionNode::getBookId() const
{
    return m_parentItem->getBookId();
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
    return m_parentItem->row();
}
