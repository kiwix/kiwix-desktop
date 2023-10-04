#include "klistwidgetitem.h"

int KListWidgetItem::m_itemHeight = 35;

KListWidgetItem::KListWidgetItem(QString text)
    : QListWidgetItem (text)
{
    setSizeHint(QSize(200, m_itemHeight));
    setBackground(QColor("white"));
    setForeground(QColor("#666666"));
}

void KListWidgetItem::disableHighlight()
{
    isHighlighted = false;
}

void KListWidgetItem::enableHighlight()
{
    isHighlighted = true;
}

QVariant KListWidgetItem::data(int role) const
{
    QVariant v = QListWidgetItem::data(role);
    if( isSelected()) {
        if (role == Qt::FontRole) {
            QFont font = v.value<QFont>();
            font.setBold( true );
            v = QVariant::fromValue<QFont>( font );
        }
    }
    if (isHighlighted) {
        if (role == Qt::BackgroundRole) {
            v = QVariant::fromValue(QColor("#4e63ad"));
        } else if (role == Qt::ForegroundRole) {
            v = QVariant::fromValue(QColor("white"));
        }
    }
    return v;
}
