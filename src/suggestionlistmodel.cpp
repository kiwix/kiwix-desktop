#include "suggestionlistmodel.h"
#include "kiwixapp.h"
#include "suggestionlistworker.h"

#include <QIcon>

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
        case Qt::DecorationRole:
            if (row < m_urlList.size())
            {
                auto library = KiwixApp::instance()->getLibrary();
                const auto zimId = m_urlList.at(row).host().split(".")[0];
                const auto defaultIcon = QIcon(":/icons/placeholder-icon.png");
                return library->getZimIcon(zimId, defaultIcon);
            }
            break;
    }
    return QVariant();
}

QVariant SuggestionListModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    if (section != 0 || orientation != Qt::Orientation::Horizontal)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            return gt("kiwix-search");
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
    setNoMoreSuggestion(urlList.size());
    endResetModel();
}

void SuggestionListModel::append(const QStringList &suggestions,
                                 const QVector<QUrl> &urlList)
{
    beginResetModel();
    if (m_hasFullText)
    {
        m_urlList.pop_back();
        m_suggestions.pop_back();
    }

    for (int i = 0; i < urlList.size(); i++)
    {
        m_urlList.append(urlList[i]);
        m_suggestions.append(suggestions[i]);
    }
    setNoMoreSuggestion(urlList.size());
    endResetModel();
}

QModelIndex SuggestionListModel::lastIndex() const
{
    return index(rowCount() - 1);
}

QModelIndex SuggestionListModel::fetchEndIndex() const
{
    int trueFetchSize = m_urlList.size() - (m_hasFullText ? 1 : 0);
    return index(trueFetchSize - 1);
}

void SuggestionListModel::setNoMoreSuggestion(int fetchedSize)
{
    int trueFetchSize = fetchedSize - (m_hasFullText ? 1 : 0);
    m_noMoreSuggestion = trueFetchSize < SuggestionListWorker::getFetchSize();
}
