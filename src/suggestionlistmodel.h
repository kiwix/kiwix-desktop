#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>

struct SuggestionData
{
    QString text;
    QUrl url;

    bool isFullTextSearchSuggestion() const;
};

class SuggestionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SuggestionListModel(QObject *parent = nullptr);
    ~SuggestionListModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void resetSuggestions();
    void append(const QList<SuggestionData>& suggestionList);

    int countOfRegularSuggestions() const;
    bool hasFullTextSearchSuggestion() const;

private:
    QList<SuggestionData> m_suggestions;
};

#endif // SUGGESTIONLISTMODEL_H
