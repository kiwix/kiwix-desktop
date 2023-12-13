#ifndef CONTENTMANAGERMODEL_H
#define CONTENTMANAGERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QIcon>
#include "thumbnaildownloader.h"
#include <memory>

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
    explicit ContentManagerModel(QObject *parent = nullptr);
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
    void setBooksData(const BookInfoList& data);
    void setupNodes();
    bool hasChildren(const QModelIndex &parent) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void refreshIcons();

    std::shared_ptr<RowNode> createNode(BookInfo bookItem, QMap<QString, QByteArray> iconMap) const;

public slots:
    void updateImage(QModelIndex index, QString url, QByteArray imageData);
    void startDownload(QModelIndex index);
    void pauseDownload(QModelIndex index);
    void resumeDownload(QModelIndex index);
    void cancelDownload(QModelIndex index);

protected: // functions
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private: // data
    BookInfoList m_data;
    std::shared_ptr<RowNode> rootNode;
    int zimCount = 0;
    ThumbnailDownloader td;
    QMap<QString, QByteArray> iconMap;
    QMap<QString, QTimer*> timers;
};

#endif // CONTENTMANAGERMODEL_H
