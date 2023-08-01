#ifndef CONTENTMANAGERDELEGATE_H
#define CONTENTMANAGERDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QByteArray>

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
    QByteArray placeholderIcon;
    void handleLastColumnClicked(const QModelIndex& index, QMouseEvent *event, const QStyleOptionViewItem &option);
};

#endif // CONTENTMANAGERDELEGATE_H
