#include "suggestionlistmodel.h"
#include "kiwixapp.h"
#include "css_constants.h"

#include <QIcon>

namespace HeaderSectionCSS = CSS::PopupCSS::QHeaderView::section;

QString getZimIdFromUrl(QUrl url);

SuggestionListModel::SuggestionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SuggestionListModel::~SuggestionListModel()
{
}

int SuggestionListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
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

    if ( role == Qt::UserRole )
            return m_suggestions.at(row).url;

    if ( index.column() == 0 && role == Qt::DecorationRole )
    {
        const auto library = KiwixApp::instance()->getLibrary();
        const auto zimId = getZimIdFromUrl(m_suggestions.at(row).url);
        return library->getBookIcon(zimId);
    }

    if ( index.column() == 1 )
    {
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return m_suggestions.at(row).text;
        case Qt::SizeHintRole:
        {
            /* Padding in css can't change height, we have to achieve padding
               by increasing height.
            */
            const int padding = HeaderSectionCSS::paddingVertical;
            const int lineHeight = HeaderSectionCSS::lineHeight;
            return QSize(0, lineHeight + 2 * padding);
        }
    }
    }
    return QVariant();
}

QVariant SuggestionListModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    if (section != 1 || orientation != Qt::Orientation::Horizontal)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            return gt("kiwix-search");
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
    if (hasFullTextSearchSuggestion())
        m_suggestions.pop_back();

    for (const auto& suggestion : suggestionList)
        m_suggestions.append(suggestion);
    endResetModel();
}

int SuggestionListModel::countOfRegularSuggestions() const
{
    return hasFullTextSearchSuggestion()
            ? rowCount() - 1
            : rowCount();
}

bool SuggestionListModel::hasFullTextSearchSuggestion() const
{
    return rowCount() > 0 && m_suggestions.last().isFullTextSearchSuggestion();
}

bool SuggestionData::isFullTextSearchSuggestion() const
{
    return url.host().endsWith(".search");
}
