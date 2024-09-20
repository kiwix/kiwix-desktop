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
    menuWidget->layout()->setSpacing(0);
    menuWidget->layout()->setContentsMargins(0, 0, 0, 0);
    menuWidget->layout()->addWidget(mp_buttonList);
    menuWidget->layout()->addWidget(mp_selectAllButton);
    
    /* TODO: connect with list widget after multi-zim */
    mp_selectAllButton->setDisabled(true);
    mp_selectAllButton->hide();
    mp_selectAllButton->setFixedHeight(24);
    mp_selectAllButton->setObjectName("selectAllButton");

    auto align = KiwixApp::isRightToLeft() ? Qt::LeftToRight : Qt::RightToLeft;
    mp_selectAllButton->setLayoutDirection(align);

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

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, bookId);
            item->setSizeHint(QSize(0, 34));

            if (current && current->zimId() == bookId)
            {
                mp_buttonList->insertItem(0, item);
                bookTitle = "*" + bookTitle;
            }
            else
                mp_buttonList->addItem(item);

            auto zimWidget = new ZimItemWidget(bookTitle, zimIcon);
            mp_radioButtonGroup->addButton(zimWidget->getRadioButton());
            mp_buttonList->setItemWidget(item, zimWidget);
        }
        catch(...)
        {
            continue;
        }
    }

    setDisabled(mp_buttonList->model()->rowCount() == 0);

    mp_buttonList->scrollToTop();
    if (auto firstWidget = getZimWidget(mp_buttonList->item(0)))
        firstWidget->getRadioButton()->setChecked(true);

    /* padding from resources/css/style.css QMultiZimButton QListWidget */
    mp_buttonList->setFixedHeight(34 * std::min(7, mp_buttonList->count()) + 10);
    mp_buttonList->setFixedWidth(menu()->width());
}

QString MultiZimButton::getZimId() const
{
    for (int row = 0; row < mp_buttonList->model()->rowCount(); row++)
    {
        auto item = mp_buttonList->item(row);
        auto widget = getZimWidget(item);
        if (widget && widget->getRadioButton()->isChecked())
            return item->data(Qt::UserRole).toString();
    }
    return QString{};
}

ZimItemWidget *MultiZimButton::getZimWidget(QListWidgetItem* item) const
{
    auto widget = mp_buttonList->itemWidget(item);
    return qobject_cast<ZimItemWidget *>(widget);
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

    QSize iconSize = QSize(24, 24);
    textLabel->setText(text);

    /* Align text on same side irregardless of text direction. */
    bool needAlignReverse = KiwixApp::isRightToLeft() == text.isRightToLeft();
    auto align = needAlignReverse ? Qt::AlignLeft : Qt::AlignRight;
    textLabel->setAlignment({Qt::AlignVCenter | align});

    iconLabel->setPixmap(icon.pixmap(iconSize));
    iconLabel->setFixedSize(iconSize);

    radioBt->setFixedSize(iconSize);

    layout()->addWidget(iconLabel);
    layout()->addWidget(textLabel);
    layout()->addWidget(radioBt);
}
