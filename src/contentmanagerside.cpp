#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"

ContentManagerSide::ContentManagerSide(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::contentmanagerside)
{
    mp_ui->setupUi(this);
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(!checked); });
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(checked); });
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->allFileButton->setStyleSheet(
                    checked ? "*{font-weight: bold}" : "");});
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->localFileButton->setStyleSheet(
                    checked ?"*{font-weight: bold}" : "");});
    mp_ui->localFileButton->setStyleSheet("*{font-weight: bold}");
}

ContentManagerSide::~ContentManagerSide()
{
    delete mp_ui;
}
