#include "klistwidgetitem.h"

int KListWidgetItem::m_itemHeight = 35;

KListWidgetItem::KListWidgetItem(QString text)
    : QListWidgetItem (text)
{
    setSizeHint(QSize(200, m_itemHeight));
}

QVariant KListWidgetItem::data(int role) const
{
    QVariant v = QListWidgetItem::data(role);
    if( isSelected() && role == Qt::FontRole )
    {
          QFont font = v.value<QFont>();
          font.setBold( true );
          v = QVariant::fromValue<QFont>( font );
    }
    return v;
}
