#ifndef ROWNODE_H
#define ROWNODE_H

#include "node.h"
#include <QList>
#include <QIcon>
#include "kiwix/book.h"

struct DownloadInfo
{
    double progress;
    QString completedLength;
    QString downloadSpeed;
    bool paused;
};

class DownloadState
{
public:
    DownloadState();

    DownloadInfo getDownloadInfo() const { return m_downloadInfo; }
    QTimer* getDownloadUpdateTimer() const { return m_downloadUpdateTimer.get(); }
    void pause();
    void resume();
    bool update(QString id);

protected:
    // This is non-NULL only for a pending (even if paused) download
    std::unique_ptr<QTimer> m_downloadUpdateTimer;

    DownloadInfo m_downloadInfo;
};

class RowNode : public Node
{
public:
    explicit RowNode(QList<QVariant> itemData, QString bookId, std::weak_ptr<RowNode> parentItem);
    ~RowNode();
    std::shared_ptr<Node> parentItem() override;
    std::shared_ptr<Node> child(int row);
    void appendChild(std::shared_ptr<Node> child);
    int childCount() const override;
    int columnCount() const override;
    QVariant data(int column) override;
    int row() const override;
    QString getBookId() const override { return m_bookId; }
    void setIconData(QByteArray iconData) { m_itemData[0] = iconData; }
    bool isChild(Node* candidate);


    void setDownloadState(std::shared_ptr<DownloadState> ds);
    std::shared_ptr<DownloadState> getDownloadState() { return m_downloadState; }

private:
    QList<QVariant> m_itemData;
    QList<std::shared_ptr<Node>> m_childItems;
    std::weak_ptr<RowNode> m_parentItem;
    QString m_bookId;
    std::shared_ptr<DownloadState> m_downloadState;
};


#endif // ROWNODE_H
