#ifndef CONTENTMANAGERDELEGATE_H
#define CONTENTMANAGERDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>

class ContentManagerDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    ContentManagerDelegate(QObject *parent=0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QScopedPointer<QPushButton> baseButton;
};

#endif // CONTENTMANAGERDELEGATE_H
