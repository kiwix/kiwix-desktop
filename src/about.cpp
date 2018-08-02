#include "about.h"
#include "ui_about.h"

#define _STR(X) #X
#define STR(X)  _STR(X)

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    auto htmlText = ui->aboutText->toHtml();
    htmlText = htmlText.replace("%%VERSION%%", STR(GIT_VERSION));
    htmlText = htmlText.replace("%%BUILD_DATE%%", STR(BUILD_DATE));
    ui->aboutText->setHtml(htmlText);
}

About::~About()
{
    delete ui;
}
