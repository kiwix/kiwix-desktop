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

    /* From SearchBarLineEdit::updateCompletion, urlList can be set before 
       the display texts of the suggestions.
    */
    return m_urlList.size();
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
        case Qt::UserRole:
            if (row < m_urlList.size())
                return m_urlList.at(row);
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

void SuggestionListModel::resetUrlList(const QVector<QUrl> &urlList)
{
    beginResetModel();
    m_urlList = urlList;
    endResetModel();
}
