#include "kiwixchoicebox.h"
#include "ui_kiwixchoicebox.h"
#include "klistwidgetitem.h"
#include "kiwixapp.h"
#include "choiceitem.h"
#include <QFile>
#include "choiceitem.h"
#include "kiwixapp.h"
#include <QFile>
#include <QDebug>
#include <QAbstractItemView>
#include <QScrollBar>
#include "kiwixlineedit.h"
#include "kiwixlistwidget.h"

KiwixChoiceBox::KiwixChoiceBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::kiwixchoicebox)
{
    ui->setupUi(this);

    QFile file(QString::fromUtf8(":/css/choiceBox.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    this->setStyleSheet(styleSheet);
    ui->clearButton->setText(gt("clear"));
    ui->clearButton->setToolTip(gt("clear-filter"));

    choiceLabel = ui->choiceLabel;
    choiceLabel->setText(gt("undefined"));

    choiceSelector = new KiwixListWidget(parent);
    choiceSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    choiceSelector->setMaximumWidth(250);
    // allow maximum 6 elements
    choiceSelector->setMaximumHeight(KListWidgetItem::getItemHeight() * 6);
    choiceSelector->setCursor(Qt::PointingHandCursor);
    choiceSelector->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    choiceSelector->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    choiceSelector->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    choiceSelector->setStyleSheet(styleSheet);
    choiceSelector->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);

    currentChoicesLayout = new FlowLayout(ui->currentChoices, 4, 2, 2);
    searcher = new KiwixLineEdit();
    searcher->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    searcher->setFixedWidth(20);
    currentChoicesLayout->addWidget(searcher);
    connect(choiceSelector, &QListWidget::itemPressed, this, [=](QListWidgetItem *item) {
        searcher->clear();
        if (item->isSelected()) {
            addSelection(item);
        } else {
            removeSelection(item);
        }
    });

    connect(searcher, &QLineEdit::textChanged, [=](QString search) {
        searcher->setStyleSheet("QLineEdit{color: #666;}");
        QFontMetrics fm = searcher->fontMetrics();
        auto w = fm.horizontalAdvance(search) + 20;
        if (w + 4 < ui->currentChoices->width()) {
            searcher->setFixedWidth(w);
            ui->currentChoices->resize(ui->currentChoices->width(), currentChoicesLayout->minimumHeightForWidth(ui->currentChoices->width()));
        }
        int visibleItems = 0;
        for (auto i = 0; i < choiceSelector->count(); i++) {
            auto itemAtRow = choiceSelector->item(i);
            itemAtRow->setHidden(!itemAtRow->text().contains(search, Qt::CaseSensitivity::CaseInsensitive));
            visibleItems += !(itemAtRow->isHidden());
        }
        choiceSelector->setVisibleItems(visibleItems);
        adjustSize();
        choiceSelector->setVisible(true);
    });

    connect(searcher, &KiwixLineEdit::clicked, [=]() {
        showOptions();
    });

    choiceSelector->setVisible(false);
    searcher->setStyleSheet("QLineEdit{color: #999;}");

    connect(searcher, &KiwixLineEdit::focusedOut, [=]() {
        hideOptions();
    });

    connect(searcher, &KiwixLineEdit::focusedIn, [=]() {
        showOptions();
    });

    ui->clearButton->setCursor(Qt::PointingHandCursor);
    connect(ui->clearButton, &QPushButton::clicked, [=]() {
        clearSelections();
        emit(choiceUpdated(getCurrentSelected()));
        hideOptions();
    });

    connect(this, &KiwixChoiceBox::choiceUpdated, [=]() {
        if (choiceSelector->selectedItems().isEmpty())
            showPlaceholder();
        choiceSelector->setVisible(false);
    });

    connect(this, &KiwixChoiceBox::clicked, [=]() {
        showOptions();
    });
}

KiwixChoiceBox::~KiwixChoiceBox()
{
    delete ui;
}

/*
When the lineEdit is currently focused,
if the outer widget is pressed, lineEdit loses focus and hides the options.
When mouseRelease event is called, it will create a flicker effect: 
    - Clicking and holding causes the lineEdit to lose focus and hide the options 
    - Release causes the options to show up again.
Showing the options on a mousePress doesn't allow this
*/
void KiwixChoiceBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit(clicked());
    }
}

void KiwixChoiceBox::hideOptions()
{
    if (choiceSelector->selectedItems().isEmpty()) {
        showPlaceholder();
    }
    searcher->setStyleSheet("QLineEdit{color: #999;}");
    choiceSelector->setVisible(false);
    ui->currentChoices->setStyleSheet("#currentChoices{border: 1px solid #ccc;}");
    searcher->clearFocus();
}

void KiwixChoiceBox::showOptions()
{
    ui->currentChoices->setStyleSheet("#currentChoices{border: 2px solid #4e63ad;}");
    adjustSize();
    choiceSelector->setVisible(true);
    choiceSelector->raise();
    searcher->setPlaceholderText("");
    searcher->setFocus();
}

void KiwixChoiceBox::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideOptions();
    } else if (event->key() == Qt::Key_Down) {
        if (!choiceSelector->isVisible())
            showOptions();
        choiceSelector->moveDown();
    } else if (event->key() == Qt::Key_Up) {
        choiceSelector->moveUp();
    } else if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        choiceSelector->selectCurrent();
    }
}

bool KiwixChoiceBox::addSelection(QListWidgetItem *item, bool updateRequired)
{
    auto key = item->text();
    auto value = item->data(Qt::UserRole).toString();
    auto chItem = new ChoiceItem(key, value);
    connect(chItem, &ChoiceItem::closeButtonClicked, [=](QString text) {
        auto selectionItems = choiceSelector->findItems(text, Qt::MatchExactly);
        if (selectionItems.size() != 1) return;
        removeSelection(selectionItems[0]);
    });
    chItem->setObjectName(key);
    currentChoicesLayout->insertWidget(ui->currentChoices->children().count() - 2, chItem);
    searcher->setFixedWidth(20);
    // put on top of list
    item = choiceSelector->takeItem(choiceSelector->row(item));
    choiceSelector->insertItem(0, item);
    item->setSelected(true);

    searcher->setFixedWidth(20);
    if (updateRequired) {
        searcher->setFocus();
        emit(choiceUpdated(getCurrentSelected()));
    }
    return true;
}

bool KiwixChoiceBox::removeSelection(QListWidgetItem *item)
{
    auto chItem = ui->currentChoices->findChild<ChoiceItem*>(item->text());
    chItem->deleteLater();
    // selected items are always shown at top, put it after the last selected item
    item->setSelected(false);
    auto selItems = choiceSelector->selectedItems();
    item = choiceSelector->takeItem(choiceSelector->row(item));
    choiceSelector->insertItem(selItems.size(), item);
    emit(choiceUpdated(getCurrentSelected()));
    return true;
}

void KiwixChoiceBox::clearSelections()
{
    for (auto &item : choiceSelector->selectedItems()) {
        item->setSelected(false);
        auto chItem  = ui->currentChoices->findChild<ChoiceItem*>(item->text());
        ui->currentChoices->layout()->removeWidget(chItem);
        delete chItem;
    }
}

void KiwixChoiceBox::showPlaceholder()
{
    searcher->clear();
    searcher->setPlaceholderText(gt(m_type + "-searcher-placeholder"));
    // Putting width based on placeholder contents
    QFontMetrics fm = searcher->fontMetrics();
    auto w = fm.boundingRect(gt(m_type + "-searcher-placeholder")).width();
    searcher->setFixedWidth(w + 20);
}

QString beautifyString(QString word)
{
    word = word.replace("_", " ");
    word[0] = word[0].toUpper();
    return word;
}

void KiwixChoiceBox::setSelections(QStringList selections, QStringList defaultSelection)
{
    SelectionList sList;
    for (const auto &sel : selections) {
        sList.append({sel, sel});
    }
    setSelections(sList, defaultSelection);
}

void KiwixChoiceBox::setSelections(SelectionList selections, QStringList defaultSelection)
{
    auto prevSelections = choiceSelector->selectedItems();
    for (auto prev : prevSelections) {
        QPair<QString, QString> prevPair = {prev->data(Qt::UserRole).toString(), prev->text()};
        if (!selections.contains(prevPair)) {
            selections.append(prevPair);
        }
    }
    clearSelections();
    choiceSelector->clear();
    for (const auto &selection: selections)
    {
        auto item = new KListWidgetItem(beautifyString(selection.second));
        item->setData(Qt::UserRole, selection.first);
        choiceSelector->addItem(item);
        if (defaultSelection.contains(selection.first)) {
            addSelection(item, false);
        }
    }
    if (choiceSelector->selectedItems().isEmpty())
        showPlaceholder();
    choiceSelector->setVisibleItems(choiceSelector->count());
    adjustSize();
}

void KiwixChoiceBox::adjustSize()
{
    QWidget::adjustSize();
    choiceSelector->adjustSize();
    choiceSelector->setGeometry(this->x() + ui->currentChoices->x(),
                                this->y() + ui->currentChoices->y() + ui->currentChoices->height(),
                                choiceSelector->width(),
                                choiceSelector->getVisibleItems() * KListWidgetItem::getItemHeight() + 2); // 2 is for the border so that all elements are visible
}

void KiwixChoiceBox::setType(QString type)
{
    ui->choiceLabel->setText(gt(type));
    m_type = type;
}

QStringList KiwixChoiceBox::getCurrentSelected()
{
    QStringList selections;
    for (auto &item : choiceSelector->selectedItems()) {
        selections.append(item->data(Qt::UserRole).toString());
    }
    return selections;
}
