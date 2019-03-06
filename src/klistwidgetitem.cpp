#include "klistwidgetitem.h"

KListWidgetItem::KListWidgetItem(QString text)
    : QListWidgetItem (text)
{

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
