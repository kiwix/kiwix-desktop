#include "tocsidebar.h"
#include "ui_tocsidebar.h"
#include "kiwixapp.h"

TocSideBar::TocSideBar(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::TocSideBar)
{
    mp_ui->setupUi(this);
    mp_findLineEdit = mp_ui->findEdit;
    connect(mp_ui->fNextButton, &QPushButton::released,
            this, &TocSideBar::findNext);
    connect(mp_ui->fPreviousButton, &QPushButton::released,
            this, &TocSideBar::findPrevious);
    connect(mp_findLineEdit, &QLineEdit::returnPressed,
            this, &TocSideBar::findNext);
}

TocSideBar::~TocSideBar()
{
    delete mp_ui;
}

void TocSideBar::postInit()
{

}

void TocSideBar::findNext()
{
    auto searchText = mp_findLineEdit->text();
    if (searchText.isEmpty())
        return;
    auto current = KiwixApp::instance()->getTabWidget()->currentWidget();
    if (!current)
        return;
    auto page = current->page();
    page->findText(searchText);
}

void TocSideBar::findPrevious()
{
    auto searchText = mp_findLineEdit->text();
    if (searchText.isEmpty())
        return;
    auto current = KiwixApp::instance()->getTabWidget()->currentWidget();
    if (!current)
        return;
    auto page = current->page();
    page->findText(searchText, QWebEnginePage::FindBackward);
}
