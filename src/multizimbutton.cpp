#include <QListWidget>
#include <QMenu>
#include <QWidgetAction>
#include <QButtonGroup>
#include <QRadioButton>
#include "kiwixapp.h"
#include "multizimbutton.h"
#include "css_constants.h"

QString getElidedText(const QFont& font, int length, const QString& text);

MultiZimButton::MultiZimButton(QWidget *parent) :
    QToolButton(parent), 
    mp_buttonList(new QListWidget),
    mp_radioButtonGroup(new QButtonGroup(this))
{
    setMenu(new QMenu(this));
    setPopupMode(QToolButton::InstantPopup);
    setDefaultAction(KiwixApp::instance()->getAction(KiwixApp::OpenMultiZimAction));
    connect(this, &QToolButton::triggered, this, &MultiZimButton::showMenu);

    const auto popupAction = new QWidgetAction(menu());
    popupAction->setDefaultWidget(mp_buttonList);
    menu()->addAction(popupAction);

    connect(mp_buttonList, &QListWidget::currentRowChanged, this, [=](int row){
        if (const auto widget = getZimWidget(row))
            widget->getRadioButton()->setChecked(true);
    });
}

void MultiZimButton::updateDisplay()
{
    mp_buttonList->clear();
    for (const auto& button : mp_radioButtonGroup->buttons())
        mp_radioButtonGroup->removeButton(button);

    const auto library = KiwixApp::instance()->getLibrary();
    const auto view = KiwixApp::instance()->getTabWidget()->currentWebView();
    QListWidgetItem* currentItem = nullptr;
    QIcon currentIcon;
    const int paddingTopBot = CSS::MultiZimButton::QListWidget::paddingVertical * 2;
    const int itemHeight = paddingTopBot + CSS::ZimItemWidget::QLabel::lineHeight;
    for (const auto& bookId : library->getBookIds())
    {
        try
        {
            library->getArchive(bookId);
        } catch (...) { continue; }

        const QString bookTitle = QString::fromStdString(library->getBookById(bookId).getTitle());
        const QIcon zimIcon = library->getBookIcon(bookId);
        
        const auto item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId);
        item->setData(Qt::DisplayRole, bookTitle);
        item->setSizeHint(QSize(0, itemHeight));

        if (view && view->zimId() == bookId)
        {
            currentItem = item;
            currentIcon = zimIcon;
            continue;
        }

        mp_buttonList->addItem(item);
        setItemZimWidget(item, bookTitle, zimIcon);
    }

    mp_buttonList->sortItems();
    if (currentItem)
    {
        mp_buttonList->insertItem(0, currentItem);

        const auto title = currentItem->data(Qt::DisplayRole).toString();
        setItemZimWidget(currentItem, "*" + title, currentIcon);
    }

    /* Display should not be used other than for sorting. */
    for (int i = 0; i < mp_buttonList->count(); i++)
        mp_buttonList->item(i)->setData(Qt::DisplayRole, QVariant());

    setDisabled(mp_buttonList->model()->rowCount() == 0);

    mp_buttonList->scrollToTop();
    mp_buttonList->setCurrentRow(0);

    /* We set a maximum display height for list. Respect padding. */
    const int listHeight = itemHeight * std::min(7, mp_buttonList->count());
    mp_buttonList->setFixedHeight(listHeight + paddingTopBot);
    mp_buttonList->setFixedWidth(menu()->width());
}

QStringList MultiZimButton::getZimIds() const
{
    QStringList idList;
    for (int row = 0; row < mp_buttonList->count(); row++)
    {
        const auto widget = getZimWidget(row);
        if (widget && widget->getRadioButton()->isChecked())
            idList.append(mp_buttonList->item(row)->data(Qt::UserRole).toString());
    }
    return idList;
}

ZimItemWidget *MultiZimButton::getZimWidget(int row) const
{
    const auto widget = mp_buttonList->itemWidget(mp_buttonList->item(row));
    return qobject_cast<ZimItemWidget *>(widget);
}

void MultiZimButton::setItemZimWidget(QListWidgetItem *item,
                                      const QString &title, const QIcon &icon)
{
    const auto zimWidget = new ZimItemWidget(title, icon);
    mp_radioButtonGroup->addButton(zimWidget->getRadioButton());
    mp_buttonList->setItemWidget(item, zimWidget);
}

ZimItemWidget::ZimItemWidget(QString text, QIcon icon, QWidget *parent) :
    QWidget(parent),
    textLabel(new QLabel(this)),
    iconLabel(new QLabel(this)),
    radioBt(new QRadioButton(this))
{
    setLayout(new QHBoxLayout);

    const int paddingHorizontal = CSS::MultiZimButton::QListWidget::item::paddingHorizontal;
    layout()->setSpacing(paddingHorizontal);
    layout()->setContentsMargins(0, 0, 0, 0);

    const int iconWidth = CSS::ZimItemWidget::QLabel::lineHeight;
    const QSize iconSize = QSize(iconWidth, iconWidth);
    iconLabel->setPixmap(icon.pixmap(iconSize));

    /* Align text on same side irregardless of text direction. */
    const bool needAlignReverse = KiwixApp::isRightToLeft() == text.isRightToLeft();
    const auto align = needAlignReverse ? Qt::AlignLeft : Qt::AlignRight;
    textLabel->setAlignment({Qt::AlignVCenter | align});

    /* Need to align checkmark with select all button. Avoid scroller from 
       changing checkmark position by always leaving out space on scroller's 
       side. Do this by align items to the other side and reducing the total
       length. textLabel is the only expandable element here.

       We set textLabel width to make sure the entire length always leave out 
       a fixed amount of white space for scroller.
    */
    layout()->setAlignment({Qt::AlignVCenter | Qt::AlignLeading});
    const auto menu = KiwixApp::instance()->getSearchBar().getMultiZimButton().menu();
    const int iconAndCheckerWidth = iconWidth * 2;
    const int totalSpacing = paddingHorizontal * 4;

    /* Add an extra border to counteract item border on one side */
    const int border = CSS::MultiZimButton::QListWidget::item::border;
    const int scrollerWidth = CSS::MultiZimButton::QScrollBar::width;
    const int contentWidthExcludeText = iconAndCheckerWidth + totalSpacing + scrollerWidth + border;
    const int labelWidth = menu->width() - contentWidthExcludeText;
    textLabel->setFixedWidth(labelWidth);

    QString elidedText = getElidedText(textLabel->font(), labelWidth, text);
    textLabel->setText(elidedText == text ? text : elidedText.trimmed() + "(...)");

    layout()->addWidget(iconLabel);
    layout()->addWidget(textLabel);
    layout()->addWidget(radioBt);
}
