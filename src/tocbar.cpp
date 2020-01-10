#include "tocbar.h"
#include "ui_tocbar.h"
#include "kiwixapp.h"

TocBar::TocBar(QWidget *parent) :
    QDockWidget(parent),
    mp_ui(new Ui::TocBar)
{
    mp_ui->setupUi(this);
    QWidget* topBorder = new QWidget(this);
    topBorder->setFixedHeight(1);
    topBorder->setStyleSheet("background-color: #ccc;");
    this->setTitleBarWidget(topBorder);
    mp_findLineEdit = mp_ui->findEdit;
    this->setMinimumHeight(44);
    this->setMaximumHeight(44);
    connect(mp_ui->hideButton, &QPushButton::released,
            this, &TocBar::findClose);
    connect(mp_ui->fNextButton, &QPushButton::released,
            this, &TocBar::findNext);
    connect(mp_ui->fPreviousButton, &QPushButton::released,
            this, &TocBar::findPrevious);
    connect(mp_findLineEdit, &QLineEdit::returnPressed,
            this, &TocBar::findNext);
}

TocBar::~TocBar()
{
    delete mp_ui;
}

void TocBar::findNext()
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

void TocBar::findPrevious()
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

void TocBar::findClose()
{
    auto current = KiwixApp::instance()->getTabWidget()->currentWidget();
    if (!current)
        return;
    auto page = current->page();
    page->findText("");
    close();
}

void TocBar::findCloseByIndex(int index)
{
    auto widget = KiwixApp::instance()->getTabWidget()->widget(index);
    if (!widget)
        return;
    auto page = widget->page();
    page->findText("");
    close();
}

void TocBar::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
        case Qt::Key_Escape:
            findClose();
            break;
        default:
            QDockWidget::keyPressEvent(event);
    }
}