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

        const auto radioBt = new QRadioButton(bookTitle);
        radioBt->setIcon(zimIcon);
        mp_radioButtonGroup->addButton(radioBt);

        mp_buttonList->addItem(item);
        mp_buttonList->setItemWidget(item, radioBt);
    }

    mp_buttonList->sortItems();

    /* Display should not be used other than for sorting. */
    for (int i = 0; i < mp_buttonList->count(); i++)
        mp_buttonList->item(i)->setData(Qt::DisplayRole, QVariant());

    setDisabled(mp_buttonList->model()->rowCount() == 0);

    const auto firstWidget = mp_buttonList->itemWidget(mp_buttonList->item(0));
    if (const auto firstBt = qobject_cast<QRadioButton *>(firstWidget))
        firstBt->setChecked(true);
}

QStringList MultiZimButton::getZimIds() const
{
    QStringList idList;
    for (int row = 0; row < mp_buttonList->count(); row++)
    {
        const auto listItem = mp_buttonList->item(row);
        const auto radioBt = qobject_cast<QRadioButton *>(mp_buttonList->itemWidget(listItem));
        if (radioBt && radioBt->isChecked())
            idList.append(listItem->data(Qt::UserRole).toString());
    }
    return idList;
}
