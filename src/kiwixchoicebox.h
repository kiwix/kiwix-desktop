#ifndef KIWIXCHOICEBOX_H
#define KIWIXCHOICEBOX_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include "flowlayout.h"
#include <QEvent>
#include <QDebug>

class ChoiceItem;
class KiwixLineEdit;

namespace Ui {
class kiwixchoicebox;
}

class KiwixChoiceBox : public QWidget
{
    Q_OBJECT

    typedef QList<QPair<QString,QString>> SelectionList;

public:
    explicit KiwixChoiceBox(QWidget *parent = nullptr);
    void setType(QString type);
    void setSelections(SelectionList selections, QStringList defaultSelection);
    void setSelections(QStringList selections, QStringList defaultSelection);
    ~KiwixChoiceBox();
    void adjustSize();

protected:
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void choiceUpdated(QStringList);

private:
    Ui::kiwixchoicebox *ui;
    QLabel *choiceLabel;
    QLineEdit *choiceSearch;
    QListWidget *choiceSelector;
    FlowLayout *currentChoicesLayout;
    KiwixLineEdit *searcher;
    QStringList getCurrentSelected();
    bool removeSelection(QString selection);
    void clearSelections();
    bool addSelection(QString key, QString value);
    QString m_type;
    bool m_sliderMoved = false;
};

#endif // KIWIXCHOICEBOX_H
