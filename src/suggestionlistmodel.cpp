#include "suggestionlistmodel.h"

SuggestionListModel::SuggestionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SuggestionListModel::~SuggestionListModel()
{
}

int SuggestionListModel::rowCount(const QModelIndex &) const
{
    return m_suggestions.size();
}

QVariant SuggestionListModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >= rowCount())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return m_suggestions.at(row).text;
        case Qt::UserRole:
            return m_suggestions.at(row).url;
    }
    return QVariant();
}

void SuggestionListModel::resetSuggestions()
{
    beginResetModel();
    m_suggestions.clear();
    endResetModel();
}

void SuggestionListModel::append(const QStringList &suggestions,
                                 const QVector<QUrl> &urlList)
{
    beginResetModel();
    for (int i = 0; i < std::min(suggestions.size(), urlList.size()); i++)
        m_suggestions.append({suggestions[i], urlList[i]});
    endResetModel();
}

QModelIndex SuggestionListModel::lastIndex() const
{
    return index(rowCount() - 1);
}
