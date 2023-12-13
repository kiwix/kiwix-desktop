#include "rownode.h"
#include <QVariant>
#include "kiwixapp.h"
#include "descriptionnode.h"

////////////////////////////////////////////////////////////////////////////////
// DowloadState
////////////////////////////////////////////////////////////////////////////////

DownloadState::DownloadState()
    : m_downloadInfo({0, "", "", false})
{
}

void DownloadState::setIsDownloading(bool val)
{
    assert(val != isDownloading());
    if ( val ) {
        m_downloadUpdateTimer.reset(new QTimer);
        m_downloadUpdateTimer->start(1000);
    } else {
        m_downloadUpdateTimer->stop();

        // Deleting the timer object immediately instead of via
        // QObject::deleteLater() seems to be safe since it is not a recipient
        // of any events that may be in the process of being delivered to it
        // from another thread.
        m_downloadUpdateTimer.reset();
        m_downloadInfo = {0, "", "", false};
    }
}

namespace
{

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

} // unnamed namespace

void DownloadState::updateDownloadStatus(QString id)
{
    auto downloadInfos = KiwixApp::instance()->getContentManager()->updateDownloadInfos(id, {"status", "completedLength", "totalLength", "downloadSpeed"});
    double percent = downloadInfos["completedLength"].toDouble() / downloadInfos["totalLength"].toDouble();
    percent *= 100;
    percent = QString::number(percent, 'g', 3).toDouble();
    auto completedLength = convertToUnits(downloadInfos["completedLength"].toString());
    auto downloadSpeed = convertToUnits(downloadInfos["downloadSpeed"].toString()) + "/s";
    m_downloadInfo = {percent, completedLength, downloadSpeed, false};
    if (!downloadInfos["status"].isValid()) {
        setIsDownloading(false); // this stops & deletes the timer
    }
}

void DownloadState::pauseDownload()
{
    m_downloadInfo.paused = true;
    m_downloadUpdateTimer->stop();
}

void DownloadState::resumeDownload()
{
    m_downloadInfo.paused = false;
    m_downloadUpdateTimer->start(1000);
}


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
