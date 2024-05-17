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

private: // functions
    void paintBookState(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const;
    void paintButton(QPainter *p, const QRect &r, QString t) const;

private:
    QScopedPointer<QPushButton> baseButton;
    void handleLastColumnClicked(const QModelIndex& index, QMouseEvent *event, const QStyleOptionViewItem &option);
};

#endif // CONTENTMANAGERDELEGATE_H
