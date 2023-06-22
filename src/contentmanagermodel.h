#ifndef CONTENTMANAGERMODEL_H
#define CONTENTMANAGERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class Node;

class ContentManagerModel : public QAbstractItemModel
{
    Q_OBJECT

public:
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
    void setBooksData(const QList<QMap<QString, QVariant>>& data);
    void setupNodes();

private:
    QList<QMap<QString, QVariant>> m_data;
    Node *rootNode;
};

#endif // CONTENTMANAGERMODEL_H
