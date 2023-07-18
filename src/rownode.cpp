#include "rownode.h"
#include <QVariant>
#include "kiwixapp.h"
#include "descriptionnode.h"
#include "kiwix/tools.h"

RowNode::RowNode(QList<QVariant> itemData, QString bookId, RowNode *parent)
    : m_itemData(itemData), m_parentItem(parent), m_bookId(bookId)
{
    m_downloadInfo = {0, "", "", false};
}

RowNode::~RowNode()
{}

void RowNode::appendChild(Node *item)
{
    m_childItems.append(item);
}

Node *RowNode::child(int row)
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

Node* RowNode::parentItem()
{
    return m_parentItem;
}

QVariant RowNode::data(int column)
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

int RowNode::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<RowNode*>(this));

    return 0;
}

RowNode* RowNode::createNode(QMap<QString, QVariant> bookItem, QMap<QString, QByteArray> iconMap, RowNode *rootNode)
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
    auto temp = new RowNode({bookIcon, bookItem["title"],
                                   bookItem["date"],
                                   QString::fromStdString(kiwix::beautifyFileSize(bookItem["size"].toULongLong())),
                                   bookItem["tags"]
                                   }, id, rootNode);
    auto tempsTemp = new DescriptionNode(bookItem["description"].toString(), temp);
    temp->appendChild(tempsTemp);
    return temp;
}
