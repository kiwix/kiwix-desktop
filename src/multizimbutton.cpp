#include <QListWidget>
#include <QMenu>
#include <QWidgetAction>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include "kiwixapp.h"
#include "multizimbutton.h"

MultiZimButton::MultiZimButton(QWidget *parent) :
    QToolButton(parent), 
    mp_buttonList(new QListWidget),
    mp_radioButtonGroup(new QButtonGroup(this)),
    mp_selectAllButton(new QCheckBox(gt("select-all"), this))
{
    setMenu(new QMenu(this));
    setPopupMode(QToolButton::InstantPopup);
    setIcon(QIcon(":/icons/filter.svg"));
    setToolTip(gt("multi-zim-search"));

    QWidget *menuWidget = new QWidget;
    menuWidget->setLayout(new QVBoxLayout);
    menuWidget->layout()->addWidget(mp_buttonList);
    menuWidget->layout()->addWidget(mp_selectAllButton);
    
    /* TODO: connect with list widget after multi-zim */
    mp_selectAllButton->setDisabled(true);
    mp_selectAllButton->hide();

    auto popupAction = new QWidgetAction(menu());
    popupAction->setDefaultWidget(menuWidget);
    menu()->addAction(popupAction);
}

void MultiZimButton::update_display()
{
    mp_buttonList->clear();
    for (auto button : mp_radioButtonGroup->buttons())
        mp_radioButtonGroup->removeButton(button);

    auto library = KiwixApp::instance()->getLibrary();
    WebView* current = KiwixApp::instance()->getTabWidget()->currentWebView();
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

            if (current && current->zimId() == bookId)
            {
                mp_buttonList->insertItem(0, item);
                bookTitle = "*" + bookTitle;
            }
            else
                mp_buttonList->addItem(item);
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

    mp_buttonList->scrollToTop();
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
