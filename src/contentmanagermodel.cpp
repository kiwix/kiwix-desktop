#include "contentmanagermodel.h"
#include "node.h"
#include<QList>
#include<QMap>
#include<QDebug>
#include <QStringList>
#include <QSize>
#include "kiwixapp.h"


ContentManagerModel::ContentManagerModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ContentManagerModel::~ContentManagerModel()
{
    delete rootNode;
}

int ContentManagerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Node*>(parent.internalPointer())->columnCount();
    return rootNode->columnCount();
}

QVariant ContentManagerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *item = static_cast<Node*>(index.internalPointer());
    const auto displayRole = role == Qt::DisplayRole;
    const auto additionalInfoRole = role == Qt::UserRole+1 && item->isAdditonal();
    if (displayRole || additionalInfoRole)
        return item->data(index.column());

    return QVariant();
}

Qt::ItemFlags ContentManagerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid() && index.parent().isValid()) {
        return defaultFlags & ~Qt::ItemIsDropEnabled & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable;
    }
    return defaultFlags;
}

QModelIndex ContentManagerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node *parentItem;

    if (!parent.isValid())
        parentItem = rootNode;
    else
        parentItem = static_cast<Node*>(parent.internalPointer());

    Node *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex ContentManagerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node *childItem = static_cast<Node*>(index.internalPointer());
    Node *parentItem = childItem->parentItem();

    if (parentItem == rootNode)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ContentManagerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return zimCount;
}

QVariant ContentManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
        case 0: return QVariant();
        case 1: return "Name";
        case 2: return "Date";
        case 3: return "Size";
        case 4: return "Content Type";
        default: return QVariant();
    }
}

void ContentManagerModel::setBooksData(const QList<QMap<QString, QVariant>>& data)
{
    m_data = data;
    rootNode = new Node({tr("Icon"), tr("Name"), tr("Date"), tr("Size"), tr("Content Type"), tr("Download")});
    setupNodes();
    emit dataChanged(QModelIndex(), QModelIndex());
}

QString convertToUnits(QString size)
{
    QStringList units = {"bytes", "KB", "MB", "GB", "TB", "PB", "EB"};
    int unitIndex = 0;
    auto bytes = size.toDouble();
    while (bytes >= 1024 && unitIndex < units.size()) {
        bytes /= 1024;
        unitIndex++;
    }

    const auto preciseBytes = QString::number(bytes, 'g', 3);
    return preciseBytes + units[unitIndex];
}

void ContentManagerModel::setupNodes()
{
    beginResetModel();
    for (auto bookItem : m_data) {
        auto name = bookItem["title"].toString();
        auto date = bookItem["date"].toString();
        auto size = convertToUnits(bookItem["size"].toString());
        auto content = bookItem["tags"].toString();
        auto id = bookItem["id"].toString();
        auto description = bookItem["description"].toString();
        auto icon = bookItem["icon"];
        const auto temp = new Node({icon, name, date, size, content, id}, rootNode, id);
        const auto tempsTemp = new Node({"", description, "", "", "", ""}, temp, "", true);
        temp->appendChild(tempsTemp);
        rootNode->appendChild(temp);
    }
    endResetModel();
}

bool ContentManagerModel::hasChildren(const QModelIndex &parent) const
{
    Node *item = static_cast<Node*>(parent.internalPointer());
    if (item)
        return !item->isAdditonal();
    return true;
}

bool ContentManagerModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;
    return (zimCount < m_data.size());
}

void ContentManagerModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;
    int remainder = m_data.size() - zimCount;
    int zimsToFetch = qMin(5, remainder);
    beginInsertRows(QModelIndex(), zimCount, zimCount + zimsToFetch - 1);
    zimCount += zimsToFetch;
    endInsertRows();
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
    KiwixApp::instance()->getContentManager()->setSortBy(sortBy, order == Qt::AscendingOrder);
}
