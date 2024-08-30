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
class KiwixListWidget;

namespace Ui {
class kiwixchoicebox;
}

class KiwixChoiceBox : public QWidget
{
    Q_OBJECT

public:
    typedef QList<QPair<QString,QString>> SelectionList;

    explicit KiwixChoiceBox(QWidget *parent = nullptr);
    void setType(QString type);
    void setSelections(SelectionList selections, SelectionList defaultSelection);
    ~KiwixChoiceBox();
    void adjustSize();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void choiceUpdated(SelectionList);
    void clicked();

private:
    Ui::kiwixchoicebox *ui;
    QLabel *choiceLabel;
    QLineEdit *choiceSearch;
    KiwixListWidget *choiceSelector;
    FlowLayout *currentChoicesLayout;
    KiwixLineEdit *searcher;
    SelectionList getCurrentSelected();
    bool removeSelection(QListWidgetItem *item);
    void clearSelections();
    bool addSelection(QListWidgetItem *item, bool updateRequired = true);
    void showOptions();
    void hideOptions();
    void showPlaceholder();
    QString m_type;
    bool m_sliderMoved = false;
};

#endif // KIWIXCHOICEBOX_H
