#include "contentmanagermodel.h"
#include "node.h"
#include "rownode.h"
#include "descriptionnode.h"
#include <zim/error.h>
#include <zim/item.h>
#include "kiwixapp.h"
#include <kiwix/tools.h>

ContentManagerModel::ContentManagerModel(ContentManager *contentMgr)
    : QAbstractItemModel(contentMgr)
    , m_contentMgr(*contentMgr)
{
    connect(&td, &ThumbnailDownloader::oneThumbnailDownloaded, this, &ContentManagerModel::updateImage);
}

ContentManagerModel::~ContentManagerModel()
{
}

int ContentManagerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Node*>(parent.internalPointer())->columnCount();
    return rootNode->columnCount();
}

namespace
{

QIcon makeIcon(const QByteArray& iconData)
{
    QPixmap pix;
    pix.loadFromData(iconData);
    return QIcon(pix);
}

QIcon createPlaceholderIcon()
{
    QImage placeholderIconFile(":/icons/placeholder-icon.png");
    QByteArray iconData;
    QBuffer buffer(&iconData);
    buffer.open(QIODevice::WriteOnly);
    placeholderIconFile.save(&buffer, "png");
    return makeIcon(iconData);
}

QIcon getIcon(const QByteArray& iconData)
{
    static QIcon placeholderIcon = createPlaceholderIcon();

    return iconData.isNull() ? placeholderIcon : makeIcon(iconData);
}

} // unnamed namespace

QVariant ContentManagerModel::data(const QModelIndex& index, int role) const
{
    if ( ! index.isValid() )
        return QVariant();

    const auto col = index.column();
    const bool isThumbnailRequest =
        !isDescriptionIndex(index) && col == 0 && role == Qt::DecorationRole;
    const bool isDescriptionRequest =
        isDescriptionIndex(index) && col == 0 && role == Qt::DisplayRole;
    const bool otherDataRequest = col != 0 && role == Qt::DisplayRole;

    if ( !isThumbnailRequest && !otherDataRequest && !isDescriptionRequest )
        return QVariant();

    const auto item = static_cast<Node*>(index.internalPointer());
    QVariant r = item->data(col);
    if ( !isThumbnailRequest )
        return r;

    r = getThumbnail(r);

    if ( r.userType() == QMetaType::QByteArray )
        return getIcon(r.toByteArray());

    const QString faviconUrl = r.toString();
    if ( !faviconUrl.isEmpty() )
        td.addDownload(faviconUrl, item->getBookId());

    return QVariant();
}

Qt::ItemFlags ContentManagerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (isDescriptionIndex(index)) {
        return defaultFlags & ~Qt::ItemIsDropEnabled & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable;
    }
    return defaultFlags;
}

QModelIndex ContentManagerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    RowNode* parentItem;

    if (!parent.isValid()) {
        parentItem = rootNode.get();
    }
    else {
        parentItem = static_cast<RowNode*>(parent.internalPointer());
    }
    auto childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem.get());

    return QModelIndex();
}

QModelIndex ContentManagerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto childItem = static_cast<Node*>(index.internalPointer());
    auto parentItem = childItem->parentItem();

    if (!parentItem || parentItem == rootNode)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem.get());
}

int ContentManagerModel::rowCount(const QModelIndex &parent) const
{
    const auto node = parent.isValid()
                    ? static_cast<const Node*>(parent.internalPointer())
                    : rootNode.get();

    return node->childCount();
}

QVariant ContentManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
        case 1: return "Name";
        case 2: return "Date";
        case 3: return "Size";
        case 4: return "Content Type";
        default: return QVariant();
    }
}

void ContentManagerModel::setBooksData(const BookInfoList& data, const Downloads& downloads)
{
    rootNode = std::shared_ptr<RowNode>(new RowNode({tr("Icon"), tr("Name"), tr("Date"), tr("Size"), tr("Content Type"), tr("Download")}, "", std::weak_ptr<RowNode>()));
    beginResetModel();
    bookIdToRowMap.clear();
    for (auto bookItem : data) {
        const auto rowNode = createNode(bookItem);

        // Restore download state during model updates (filtering, etc)
        rowNode->setDownloadState(downloads.value(rowNode->getBookId()));

        bookIdToRowMap[bookItem["id"].toString()] = rootNode->childCount();
        rootNode->appendChild(rowNode);
    }
    endResetModel();
    emit dataChanged(QModelIndex(), QModelIndex());
}

// Returns either data of the thumbnail (as a QByteArray) or a URL (as a
// QString) from where the actual data can be obtained.
QVariant ContentManagerModel::getThumbnail(const QVariant& faviconEntry) const
{
    if ( faviconEntry.userType() == QMetaType::QByteArray )
        return faviconEntry;

    const auto faviconUrl = faviconEntry.toString();
    return m_iconMap.contains(faviconUrl)
         ? m_iconMap[faviconUrl]
         : faviconEntry;
}

std::shared_ptr<RowNode> ContentManagerModel::createNode(BookInfo bookItem) const
{
    QString id = bookItem["id"].toString();
    const QVariant bookIcon = getThumbnail(bookItem["favicon"]);
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

bool ContentManagerModel::hasChildren(const QModelIndex &parent) const
{
    auto item = static_cast<Node*>(parent.internalPointer());
    if (item)
        return item->childCount() > 0;
    return true;
}

void ContentManagerModel::sort(int column, Qt::SortOrder order)
{
    if (column == 0 || column == 4 || column == 5)
        return;

    QString sortBy = "";
    switch(column) {
        case 1:
            sortBy = "title";
            break;
        case 2:
            sortBy = "date";
            break;
        case 3:
            sortBy = "size";
            break;
        default:
            sortBy = "unsorted";
    }
    m_contentMgr.setSortBy(sortBy, order == Qt::AscendingOrder);
}

RowNode* ContentManagerModel::getRowNode(size_t row)
{
    return static_cast<RowNode*>(rootNode->child(row).get());
}

void ContentManagerModel::updateImage(QString bookId, QString url, QByteArray imageData)
{
    const auto it = bookIdToRowMap.constFind(bookId);
    if ( it == bookIdToRowMap.constEnd() )
        return;

    const size_t row = it.value();
    const auto item = getRowNode(row);
    item->setIconData(imageData);
    m_iconMap[url] = imageData;
    triggerDataUpdateAt( this->index(row, 0) );
}

void ContentManagerModel::updateDownload(QString bookId)
{
    const auto it = bookIdToRowMap.constFind(bookId);

    if ( it != bookIdToRowMap.constEnd() ) {
        const size_t row = it.value();
        triggerDataUpdateAt( this->index(row, 5) );
    }
}

void ContentManagerModel::triggerDataUpdateAt(QModelIndex index)
{
    emit dataChanged(index, index);
}

void ContentManagerModel::removeDownload(QString bookId)
{
    const auto it = bookIdToRowMap.constFind(bookId);
    if ( it == bookIdToRowMap.constEnd() )
        return;

    const size_t row = it.value();
    getRowNode(row)->setDownloadState(nullptr);
    triggerDataUpdateAt( this->index(row, 5) );
}

