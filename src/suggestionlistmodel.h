#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>

class SuggestionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SuggestionListModel(QObject *parent = nullptr);
    ~SuggestionListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void resetSuggestions(const QStringList& suggestions = QStringList{});
    void resetUrlList(const QVector<QUrl>& urlList = QVector<QUrl>{});

private:
    QStringList m_suggestions;
    QVector<QUrl> m_urlList;
};

#endif // SUGGESTIONLISTMODEL_H
