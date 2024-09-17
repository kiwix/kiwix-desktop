#ifndef SUGGESTIONLISTMODEL_H
#define SUGGESTIONLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>

class SuggestionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SuggestionListModel(QObject *parent = nullptr);
    ~SuggestionListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void resetSuggestions(const QStringList& suggestions = QStringList{});
    void resetUrlList(const QVector<QUrl>& urlList = QVector<QUrl>{});
    void append(const QStringList& suggestions, const QVector<QUrl>& urlList);

    QModelIndex lastIndex() const;
    QModelIndex fetchEndIndex() const;
    void setHasFullText(bool hasFullText) { m_hasFullText = hasFullText; }
    bool hasFullText() const { return m_hasFullText; }
    bool noMoreSuggestion() const { return m_noMoreSuggestion; }

private:
    QStringList m_suggestions;
    QVector<QUrl> m_urlList;
    bool m_hasFullText = true;
    bool m_noMoreSuggestion = true;

    void setNoMoreSuggestion(int fetchedSize);
};

#endif // SUGGESTIONLISTMODEL_H
