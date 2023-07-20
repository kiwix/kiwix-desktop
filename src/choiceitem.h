#ifndef CHOICEITEM_H
#define CHOICEITEM_H

#include <QWidget>

namespace Ui {
class ChoiceItem;
}

class ChoiceItem : public QWidget
{
    Q_OBJECT

public:
    explicit ChoiceItem(QString key, QString value, QWidget *parent = nullptr);
    ~ChoiceItem();
    QString getKey() { return m_key; }
    QString getValue() { return m_value; }

private:
    Ui::ChoiceItem *ui;
    QString m_key;
    QString m_value;

signals:
    void closeButtonClicked(QString);
};

#endif // CHOICEITEM_H
