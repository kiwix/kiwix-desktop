#include "suggestionlistmodel.h"

SuggestionListModel::SuggestionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SuggestionListModel::~SuggestionListModel()
{
}

int SuggestionListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

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

void SuggestionListModel::append(const QList<SuggestionData> &suggestionList)
{
    beginResetModel();
    for (const auto& suggestion : suggestionList)
        m_suggestions.append(suggestion);
    endResetModel();
}
