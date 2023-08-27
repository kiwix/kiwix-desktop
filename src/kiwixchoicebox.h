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
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void choiceUpdated(QStringList);
    void clicked();

private:
    Ui::kiwixchoicebox *ui;
    QLabel *choiceLabel;
    QLineEdit *choiceSearch;
    KiwixListWidget *choiceSelector;
    FlowLayout *currentChoicesLayout;
    KiwixLineEdit *searcher;
    QStringList getCurrentSelected();
    bool removeSelection(QListWidgetItem *item);
    void clearSelections();
    bool addSelection(QListWidgetItem *item);
    void showOptions();
    void hideOptions();
    QString m_type;
    bool m_sliderMoved = false;
};

#endif // KIWIXCHOICEBOX_H
