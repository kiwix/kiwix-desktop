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
        case Qt::DecorationRole:
        {
            // XXX: This is a hack. All displayable data of the suggestion is
            // XXX: returned via UserRole to avoid problems with custom
            // XXX: rendering (which is performed via SuggestionListDelegate)
            return QVariant();
        }
        case Qt::UserRole:
        {
            const SuggestionData suggestionData = m_suggestions.at(row);
            UserData ud;
            ud.text = suggestionData.text;
            ud.url =  suggestionData.url;
            const auto library = KiwixApp::instance()->getLibrary();
            const auto zimId = getZimIdFromUrl(m_suggestions.at(row).url);
            ud.icon = library->getBookIcon(zimId);
            QVariant ret;
            ret.setValue(ud);
            return ret;
        }
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
