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
    setIcon(QIcon(":/icons/filter.svg"));
    setToolTip(gt("multi-zim-search"));

    auto popupAction = new QWidgetAction(menu());
    popupAction->setDefaultWidget(mp_buttonList);
    menu()->addAction(popupAction);
}

void MultiZimButton::update_display()
{
    mp_buttonList->clear();
    for (auto button : mp_radioButtonGroup->buttons())
        mp_radioButtonGroup->removeButton(button);

    auto library = KiwixApp::instance()->getLibrary();
    for (const auto& bookId : library->getBookIds())
    {
        std::shared_ptr<zim::Archive> archive;
        QString bookTitle;
        QIcon zimIcon;
        try
        {
            archive = library->getArchive(bookId);
            bookTitle = QString::fromStdString(library->getBookById(bookId).getTitle());
            zimIcon = library->getZimIcon(bookId, QIcon(":/icons/placeholder-icon.png"));

            QRadioButton* radioBt = new QRadioButton;
            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, bookId);
            radioBt->setIcon(zimIcon);
            radioBt->setText(bookTitle);
            mp_radioButtonGroup->addButton(radioBt);

            mp_buttonList->addItem(item);
            mp_buttonList->setItemWidget(item, radioBt);
        }
        catch(...)
        {
            continue;
        }
    }

    setDisabled(mp_buttonList->model()->rowCount() == 0);

    auto firstWidget = mp_buttonList->itemWidget(mp_buttonList->item(0));
    if (auto firstBt = qobject_cast<QRadioButton *>(firstWidget))
        firstBt->setChecked(true);
}

QString MultiZimButton::getZimId() const
{
    for (int row = 0; row < mp_buttonList->model()->rowCount(); row++)
    {
        auto listItem = mp_buttonList->item(row);
        auto radioBt = qobject_cast<QRadioButton *>(mp_buttonList->itemWidget(listItem));
        if (radioBt && radioBt->isChecked())
            return listItem->data(Qt::UserRole).toString();
    }
    return QString{};
}
