#ifndef SUGGESTIONLISTDELEGATE_H
#define SUGGESTIONLISTDELEGATE_H

#include <QStyledItemDelegate>

class SuggestionListDelegate : public QStyledItemDelegate
{
public:
    SuggestionListDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {};
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintIcon(QPainter *p, const QStyleOptionViewItem &opt, const QIcon &icon) const;
    void paintText(QPainter *p, const QStyleOptionViewItem &opt, const QString &text) const;
};

#endif // SUGGESTIONLISTDELEGATE_H
