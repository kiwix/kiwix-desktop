#include "rownode.h"
#include <QVariant>
#include "kiwixapp.h"
#include "descriptionnode.h"
#include "kiwix/tools.h"

RowNode::RowNode(QList<QVariant> itemData, QString bookId, std::weak_ptr<RowNode> parent)
    : m_itemData(itemData), m_parentItem(parent), m_bookId(bookId)
{
    m_downloadInfo = {0, "", "", false};
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

std::shared_ptr<RowNode> RowNode::createNode(QMap<QString, QVariant> bookItem, QMap<QString, QByteArray> iconMap, std::shared_ptr<RowNode> rootNode)
{
    auto faviconUrl = "https://" + bookItem["faviconUrl"].toString();
    QString id = bookItem["id"].toString();
    QByteArray bookIcon;
    try {
        auto book = KiwixApp::instance()->getLibrary()->getBookById(id);
        std::string favicon;
        auto item = book.getIllustration(48);
        favicon = item->getData();
        bookIcon = QByteArray::fromRawData(reinterpret_cast<const char*>(favicon.data()), favicon.size());
        bookIcon.detach(); // deep copy
    } catch (...) {
        if (iconMap.contains(faviconUrl)) {
            bookIcon = iconMap[faviconUrl];
        }
    }
    std::weak_ptr<RowNode> weakRoot = rootNode;
    auto rowNodePtr = std::shared_ptr<RowNode>(new
                                    RowNode({bookIcon, bookItem["title"],
                                   bookItem["date"],
                                   QString::fromStdString(kiwix::beautifyFileSize(bookItem["size"].toULongLong())),
                                   bookItem["tags"]
                                   }, id, weakRoot));
    std::weak_ptr<RowNode> weakRowNodePtr = rowNodePtr;
    const auto descNodePtr = std::make_shared<DescriptionNode>(DescriptionNode(bookItem["description"].toString(), weakRowNodePtr));
    rowNodePtr->appendChild(descNodePtr);
    return rowNodePtr;
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
