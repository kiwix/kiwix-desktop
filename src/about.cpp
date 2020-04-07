#include "about.h"
#include "ui_about.h"
#include "kiwixapp.h"

#define _STR(...) # __VA_ARGS__
#define STR(X) _STR(X)

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    auto htmlText = ui->aboutText->toHtml();
    htmlText = htmlText.replace("%%KIWIX_DESKTOP_DESCRIPTION%%", gt("kiwix-desktop-description"));
    htmlText = htmlText.replace("%%LEARN_MORE_ABOUT_KIWIX%%", gt("learn-more-about-kiwix"));
    htmlText = htmlText.replace("%%RELEASE%%", gt("release"));
    htmlText = htmlText.replace("%%SOURCE_CODE%%", gt("source-code"));
    htmlText = htmlText.replace("%%VERSION_LABEL%%", gt("version"));
    htmlText = htmlText.replace("%%VERSION%%", STR(GIT_VERSION));
    htmlText = htmlText.replace("%%BUILD_DATE_LABEL%%", gt("build-date"));
    htmlText = htmlText.replace("%%BUILD_DATE%%", STR(BUILD_DATE));
    htmlText = htmlText.replace("%%REPORTING_PROBLEM%%", gt("reporting-problem"));
    htmlText = htmlText.replace("%%REPORT_ISSUE_1%%", gt("report-issue-1/3"));
    htmlText = htmlText.replace("%%REPORT_ISSUE_2%%", gt("report-issue-2/3"));
    htmlText = htmlText.replace("%%REPORT_ISSUE_3%%", gt("report-issue-3/3"));
    htmlText = htmlText.replace("%%LIBRARIES%%", gt("Libraries"));
    ui->aboutText->setHtml(htmlText);
}

About::~About()
{
    delete ui;
}
