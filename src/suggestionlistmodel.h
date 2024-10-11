#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>

struct SuggestionData
{
    QString text;
    QUrl url;
};

class SuggestionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SuggestionListModel(QObject *parent = nullptr);
    ~SuggestionListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void resetSuggestions();
    void append(const QList<SuggestionData>& suggestionList);

private:
    QList<SuggestionData> m_suggestions;
};

#endif // SUGGESTIONLISTMODEL_H
