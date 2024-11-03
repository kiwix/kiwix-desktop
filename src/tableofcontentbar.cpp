#include "tableofcontentbar.h"
#include "ui_tableofcontentbar.h"
#include "kiwixapp.h"

TableOfContentBar::TableOfContentBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::tableofcontentbar)
{
    ui->setupUi(this);
    ui->titleLabel->setText(gt("table-of-content"));
}

TableOfContentBar::~TableOfContentBar()
{
    delete ui;
}
