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
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (row < m_suggestions.size())
                return m_suggestions.at(row);
            break;
    }
    return QVariant();
}

void SuggestionListModel::resetSuggestions(const QStringList &suggestions)
{
    beginResetModel();
    m_suggestions.clear();
    for (const auto& suggestion : suggestions)
        m_suggestions.append(suggestion);
    endResetModel();
}
