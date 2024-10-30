#include <QListWidget>
#include <QMenu>
#include <QWidgetAction>
#include <QButtonGroup>
#include <QRadioButton>
#include "kiwixapp.h"
#include "multizimbutton.h"

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
    if (const auto firstWidget = getZimWidget(0))
        firstWidget->getRadioButton()->setChecked(true);
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
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);

    textLabel->setText(text);

    /* TODO: change temporary values once size is defined laters */
    const QSize iconSize = QSize(24, 24);
    iconLabel->setPixmap(icon.pixmap(iconSize));

    layout()->addWidget(iconLabel);
    layout()->addWidget(textLabel);
    layout()->addWidget(radioBt);
}
