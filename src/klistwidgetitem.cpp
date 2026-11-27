#include "klistwidgetitem.h"
#include "kiwixapp.h"

int KListWidgetItem::m_itemHeight = 35;

KListWidgetItem::KListWidgetItem(QString text)
    : QListWidgetItem (text)
{
    setSizeHint(QSize(200, m_itemHeight));
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
    } else if (role == Qt::BackgroundRole || role == Qt::ForegroundRole) {
        // Resolved at paint time so that the colors follow theme changes.
        v = QVariant::fromValue(themeColor(role == Qt::BackgroundRole ? "SURFACE" : "CHOICE_TEXT"));
    }
    return v;
}
