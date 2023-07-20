#include "choiceitem.h"
#include "ui_choiceitem.h"
#include <QFile>

ChoiceItem::ChoiceItem(QString key, QString value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChoiceItem),
    m_key(key),
    m_value(value)
{
    ui->setupUi(this);
    ui->itemLabel->setText(key);
    ui->itemLabel->setToolTip(key);
    connect(ui->closeButton, &QPushButton::clicked, [=](){
       emit(closeButtonClicked(ui->itemLabel->text()));
    });
    ui->closeButton->setCursor(Qt::PointingHandCursor);
}

ChoiceItem::~ChoiceItem()
{
    delete ui;
}
