#include "rownode.h"
#include <QVariant>
#include "kiwixapp.h"
#include "descriptionnode.h"

////////////////////////////////////////////////////////////////////////////////
// DowloadState
////////////////////////////////////////////////////////////////////////////////

namespace
{

QString convertToUnits(double bytes)
{
    QStringList units = {"bytes", "KB", "MB", "GB", "TB", "PB", "EB"};
    int unitIndex = 0;
    while (bytes >= 1024 && unitIndex < units.size()) {
        bytes /= 1024;
        unitIndex++;
    }

    const auto preciseBytes = QString::number(bytes, 'g', 3);
    return preciseBytes + " " + units[unitIndex];
}

} // unnamed namespace

void DownloadState::update(const DownloadInfo& downloadInfos)
{
    double percent = downloadInfos["completedLength"].toDouble() / downloadInfos["totalLength"].toDouble();
    percent *= 100;
    percent = QString::number(percent, 'g', 3).toDouble();
    auto completedLength = convertToUnits(downloadInfos["completedLength"].toDouble());
    auto downloadSpeed = convertToUnits(downloadInfos["downloadSpeed"].toDouble()) + "/s";
    *this = {percent, completedLength, downloadSpeed, false};
}

void DownloadState::pause()
{
    this->paused = true;
}

void DownloadState::resume()
{
    this->paused = false;
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

void RowNode::setDownloadState(std::shared_ptr<DownloadState> ds)
{
    m_downloadState = ds;
}
