#ifndef CONTENTMANAGERMODEL_H
#define CONTENTMANAGERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QIcon>
#include "thumbnaildownloader.h"
#include "rownode.h"
#include "downloadmanagement.h"
#include <memory>

class ContentManager;
class RowNode;
class Node;
class DescriptionNode;

class ContentManagerModel : public QAbstractItemModel
{
    Q_OBJECT

public: // types
    typedef QMap<QString, QVariant> BookInfo;
    typedef QList<BookInfo>         BookInfoList;

public: // functions
    explicit ContentManagerModel(ContentManager* contentMgr);
    ~ContentManagerModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    void setBooksData(const BookInfoList& data, const DownloadManager& downloadMgr);
    bool hasChildren(const QModelIndex &parent) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    std::shared_ptr<RowNode> createNode(BookInfo bookItem) const;

public slots:
    void updateImage(QString bookId, QString url, QByteArray imageData);
    void triggerDataUpdateAt(QModelIndex index);
    void setDownloadState(QString bookId, std::shared_ptr<DownloadState> ds);
    void updateDownload(QString bookId);

private: // functions
    // Returns either data of the thumbnail (as a QByteArray) or a URL (as a
    // QString) from where the actual data can be obtained.
    QVariant getThumbnail(const QVariant& faviconEntry) const;
    RowNode* getRowNode(size_t row);

private: // data
    ContentManager& m_contentMgr;
    std::shared_ptr<RowNode> rootNode;
    mutable ThumbnailDownloader td;
    QMap<QString, size_t> bookIdToRowMap;
    QMap<QString, QByteArray> m_iconMap;
};

inline bool isDescriptionIndex(const QModelIndex& index)
{
    return index.parent().isValid();
}

#endif // CONTENTMANAGERMODEL_H
