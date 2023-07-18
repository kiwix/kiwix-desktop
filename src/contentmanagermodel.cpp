#include "contentmanagermodel.h"
#include "node.h"
#include "rownode.h"
#include "descriptionnode.h"
#include<QList>
#include<QMap>
#include<QDebug>
#include <QStringList>
#include <QSize>
#include <QIcon>
#include <zim/error.h>
#include <zim/item.h>
#include "kiwixapp.h"

ContentManagerModel::ContentManagerModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    connect(&td, &ThumbnailDownloader::oneThumbnailDownloaded, this, &ContentManagerModel::updateImage);
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
    const auto additionalInfoRole = role == Qt::UserRole+1;
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

    RowNode* parentItem;

    if (!parent.isValid()) {
        parentItem = rootNode;
    }
    else {
        parentItem = static_cast<RowNode*>(parent.internalPointer());
    }
    auto childItem = parentItem->child(row);
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
    rootNode = new RowNode({tr("Icon"), tr("Name"), tr("Date"), tr("Size"), tr("Content Type"), tr("Download")}, "", nullptr);
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
    return preciseBytes + " " + units[unitIndex];
}

void ContentManagerModel::setupNodes()
{
    beginResetModel();
    for (auto bookItem : m_data) {
        rootNode->appendChild(RowNode::createNode(bookItem, iconMap, rootNode));
    }
    endResetModel();
}

void ContentManagerModel::refreshIcons()
{
    if (KiwixApp::instance()->getContentManager()->isLocal())
        return;
    td.clearQueue();
    for (auto i = 0; i < rowCount() && i < m_data.size(); i++) {
        auto bookItem = m_data[i];
        auto id = bookItem["id"].toString();
        auto faviconUrl = "https://" + bookItem["faviconUrl"].toString();
        auto app = KiwixApp::instance();
        try {
            auto book = app->getLibrary()->getBookById(id);
            auto item = book.getIllustration(48);
        } catch (...) {
            if (faviconUrl != "" && !iconMap.contains(faviconUrl)) {
                td.addDownload(faviconUrl, index(i, 0));
            }
        }
    }
}

bool ContentManagerModel::hasChildren(const QModelIndex &parent) const
{
    Node *item = static_cast<Node*>(parent.internalPointer());
    if (item)
        return item->childCount() > 0;
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
    refreshIcons();
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

void ContentManagerModel::updateImage(QModelIndex index, QString url, QByteArray imageData)
{
    RowNode *item = static_cast<RowNode*>(index.internalPointer());
    item->setIconData(imageData);
    iconMap[url] = imageData;
    emit dataChanged(index, index);
}

void ContentManagerModel::startDownload(QModelIndex index)
{
    auto node = static_cast<RowNode*>(index.internalPointer());
    node->setIsDownloading(true);
    auto id = node->getBookId();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        auto downloadInfos = KiwixApp::instance()->getContentManager()->updateDownloadInfos(id, {"status", "completedLength", "totalLength", "downloadSpeed"});
        double percent = (double) downloadInfos["completedLength"].toInt() / downloadInfos["totalLength"].toInt();
        percent *= 100;
        percent = QString::number(percent, 'g', 3).toDouble();
        auto completedLength = convertToUnits(downloadInfos["completedLength"].toString());
        auto downloadSpeed = convertToUnits(downloadInfos["downloadSpeed"].toString()) + "/s";
        node->setDownloadInfo({percent, completedLength, downloadSpeed});
        if (!downloadInfos["status"].isValid()) {
            node->setIsDownloading(false);
            timer->stop();
            timer->deleteLater();
        }
        emit dataChanged(index, index);
    });
    timer->start(1000);
    timers[id] = timer;
}

void ContentManagerModel::pauseDownload(QModelIndex index)
{
    auto node = static_cast<RowNode*>(index.internalPointer());
    auto id = node->getBookId();
    auto prevDownloadInfo = node->getDownloadInfo();
    prevDownloadInfo.paused = true;
    node->setDownloadInfo(prevDownloadInfo);
    timers[id]->stop();
    emit dataChanged(index, index);
}

void ContentManagerModel::resumeDownload(QModelIndex index)
{
    auto node = static_cast<RowNode*>(index.internalPointer());
    auto id = node->getBookId();
    auto prevDownloadInfo = node->getDownloadInfo();
    prevDownloadInfo.paused = false;
    node->setDownloadInfo(prevDownloadInfo);
    timers[id]->start(1000);
    emit dataChanged(index, index);
}

void ContentManagerModel::cancelDownload(QModelIndex index)
{
    auto node = static_cast<RowNode*>(index.internalPointer());
    auto id = node->getBookId();
    node->setIsDownloading(false);
    node->setDownloadInfo({0, "", "", false});
    timers[id]->stop();
    timers[id]->deleteLater();
    emit dataChanged(index, index);
}

