#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"

ContentManagerSide::ContentManagerSide(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::contentmanagerside)
{
    mp_ui->setupUi(this);
}

ContentManagerSide::~ContentManagerSide()
{
    delete mp_ui;
}
