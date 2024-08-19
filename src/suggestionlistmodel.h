#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>

struct SuggestionData
{
    QString text;
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
    void append(const QStringList& suggestionList);

private:
    QList<SuggestionData> m_suggestions;
};

#endif // SUGGESTIONLISTMODEL_H
