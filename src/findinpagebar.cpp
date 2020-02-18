#include "findinpagebar.h"
#include "ui_findinpagebar.h"

FindInPageBar::FindInPageBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindInPageBar)
{
    ui->setupUi(this);
}

FindInPageBar::~FindInPageBar()
{
    delete ui;
}
