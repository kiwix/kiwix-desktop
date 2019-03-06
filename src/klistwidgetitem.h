#ifndef KLISTWIDGETITEM_H
#define KLISTWIDGETITEM_H

#include <QListWidgetItem>

class KListWidgetItem : public QListWidgetItem
{
public:
    KListWidgetItem(QString text);
    QVariant data(int role) const;
};

#endif // KLISTWIDGETITEM_H
