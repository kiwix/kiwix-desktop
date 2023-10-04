#ifndef KLISTWIDGETITEM_H
#define KLISTWIDGETITEM_H

#include <QListWidgetItem>

class KListWidgetItem : public QListWidgetItem
{
public:
    KListWidgetItem(QString text);
    QVariant data(int role) const;
    static int getItemHeight() { return m_itemHeight; };
    void disableHighlight();
    void enableHighlight();
private:
    static int m_itemHeight;
    bool isHighlighted = false;
};

#endif // KLISTWIDGETITEM_H
