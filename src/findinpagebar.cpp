#include "kiwixapp.h"
#include "findinpagebar.h"
#include "ui_findinpagebar.h"

FindInPageBar::FindInPageBar(QWidget *parent) :
    QFrame(parent),
    mp_ui(new Ui::FindInPageBar)
{
    mp_ui->setupUi(this);
    mp_findLineEdit = mp_ui->findEdit;
    connect(mp_ui->hideButton, &QPushButton::released,
            this, &FindInPageBar::findClose);
    connect(mp_ui->fNextButton, &QPushButton::released,
            this, &FindInPageBar::findNext);
    connect(mp_ui->fPreviousButton, &QPushButton::released,
            this, &FindInPageBar::findPrevious);
    connect(mp_findLineEdit, &QLineEdit::returnPressed,
            this, &FindInPageBar::findNext);
}

FindInPageBar::~FindInPageBar()
{
    delete mp_ui;
}

void FindInPageBar::findNext()
{
    auto searchText = mp_findLineEdit->text();
    if (searchText.isEmpty())
        return;
    auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;
    auto page = current->page();
    page->findText(searchText);
}

void FindInPageBar::findPrevious()
{
    auto searchText = mp_findLineEdit->text();
    if (searchText.isEmpty())
        return;
    auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;
    auto page = current->page();
    page->findText(searchText, QWebEnginePage::FindBackward);
}

void FindInPageBar::findClose()
{
    auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;
    auto page = current->page();
    page->findText("");
    close();
}

void FindInPageBar::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
        case Qt::Key_Escape:
            findClose();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}
