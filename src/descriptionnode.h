#ifndef DESCRIPTIONNODE_H
#define DESCRIPTIONNODE_H

#include<QString>
#include "node.h"

class RowNode;

class DescriptionNode : public Node
{
public:
    DescriptionNode(QString desc, std::weak_ptr<RowNode> parent);
    ~DescriptionNode();
    std::shared_ptr<Node> parentItem() override;
    int childCount() const override;
    int columnCount() const override;
    QVariant data(int column) override;
    int row() const override;
    QString getBookId() const override;

private:
    QString m_desc;
    std::weak_ptr<RowNode> m_parentItem;
};

#endif // DESCRIPTIONNODE_H
