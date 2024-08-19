#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>

class SuggestionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SuggestionListModel(QObject *parent = nullptr);
    ~SuggestionListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void resetSuggestions(const QStringList& suggestions = QStringList{});

private:
    QStringList m_suggestions;
};

#endif // SUGGESTIONLISTMODEL_H
