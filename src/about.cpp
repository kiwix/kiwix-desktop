#include "about.h"
#include "ui_about.h"
#include "kiwixapp.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    auto htmlText = ui->aboutText->toHtml();
    htmlText = htmlText.replace("{{KIWIX_DESKTOP_TITLE}}", gt("about-kiwix-desktop-title"));
    htmlText = htmlText.replace("{{KIWIX_DESKTOP_DESCRIPTION}}", gt("about-kiwix-desktop-description"));
    htmlText = htmlText.replace("{{LEARN_MORE_ABOUT_KIWIX}}", gt("about-learn-more-about-kiwix"));
    htmlText = htmlText.replace("{{RELEASE}}", gt("about-release-title"));
    htmlText = htmlText.replace("{{SOURCE_CODE}}", gt("about-source-code"));
    htmlText = htmlText.replace("{{VERSION_TXT}}", gt("about-version"));
    htmlText = htmlText.replace("{{REPORTING_PROBLEM}}", gt("about-reporting-problem-title"));
    htmlText = htmlText.replace("{{REPORT_ISSUE}}", gt("about-report-issue"));
    htmlText = htmlText.replace("{{REPORT_ISSUE_2}}", gt("about-report-issue-2"));
    htmlText = htmlText.replace("{{LIBRARIES}}", gt("about-libraries-title"));

    htmlText = htmlText.replace("{{GITHUB_URL}}", "https://github.com/kiwix/kiwix-desktop");
    htmlText = htmlText.replace("{{VERSION}}", version);
    htmlText = htmlText.replace("{{TRACKER_URL}}", "https://github.com/kiwix/kiwix-desktop/issues");
    ui->aboutText->setHtml(htmlText);
}

About::~About()
{
    delete ui;
}
