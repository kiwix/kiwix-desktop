#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"
#include "kiwixapp.h"

#include <QLocale>

#include "klistwidgetitem.h"
#include "static_content.h"

ContentManagerSide::ContentManagerSide(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::contentmanagerside)
{
    mp_ui->setupUi(this);
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(!checked); });
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(checked); });
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->allFileButton->setStyleSheet(
                    checked ? "*{font-weight: bold}" : "");});
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->localFileButton->setStyleSheet(
                    checked ?"*{font-weight: bold}" : "");});
    mp_ui->localFileButton->setStyleSheet("*{font-weight: bold}");

    mp_ui->allFileButton->setText(gt("all-files"));
    mp_ui->localFileButton ->setText(gt("local-files"));
    mp_ui->languageButton->setText(gt("browse-by-language"));
    mp_ui->categoryButton->setText(gt("browse-by-category"));
    mp_languageButton = mp_ui->languageButton;
    mp_languageSelector = mp_ui->languageSelector;
    connect(mp_languageButton, &QCheckBox::toggled, this, [=](bool checked) { mp_languageSelector->setHidden(!checked); });
    mp_languageSelector->setHidden(true);
    mp_categoryButton = mp_ui->categoryButton;
    mp_categorySelector = mp_ui->categorySelector;
    connect(mp_categoryButton, &QCheckBox::toggled, this, [=](bool checked) { mp_categorySelector->setHidden(!checked); });
    mp_categorySelector->setHidden(true);
    mp_ui->contentTypeButton->hide();

    for(auto lang: S_LANGUAGES)
    {
        auto currentLang = QLocale().language();
        auto locale = QLocale(lang);
        if (locale.language() != lang) {
            // Qt may not find the locale for the lang :/
            // In this case, Qt return the current locale
            // So we must be sure that the locale found correspond to the lang we want to add,
            // else we may add several time the current language.
            continue;
        }
        auto item = new KListWidgetItem(QLocale::languageToString(locale.language()));
        item->setData(Qt::UserRole, lang);
        mp_languageSelector->addItem(item);
        if (lang == currentLang) {
            item->setSelected(true);
        }
    }
    mp_languageSelector->sortItems();
    auto item = new KListWidgetItem("All");
    item->setData(Qt::UserRole, QLocale::AnyLanguage);
    mp_languageSelector->insertItem(0, item);

    for (auto category: S_CATEGORIES)
    {
        auto item = new KListWidgetItem(category.second);
        item->setData(Qt::UserRole, category.first);
        mp_categorySelector->addItem(item);
        if (category.first ==  "all")
        {
            item->setSelected(true);
        }
    }

}

ContentManagerSide::~ContentManagerSide()
{
    delete mp_ui;
}


void ContentManagerSide::setContentManager(ContentManager *contentManager)
{
    mp_contentManager = contentManager;
    connect(mp_languageSelector, &QListWidget::itemSelectionChanged,
            this, [=]() {
                auto item = mp_languageSelector->selectedItems().at(0);
                if (!item) return;
                auto langId = item->data(Qt::UserRole).toInt();
                auto lang = QLocale::Language(langId);
                if (lang == QLocale::AnyLanguage) {
                    mp_contentManager->setCurrentLanguage("*");
                    return;
                }
                auto locale = QLocale(lang);
                mp_contentManager->setCurrentLanguage(locale.name().split("_").at(0));
    });
    connect(mp_categorySelector, &QListWidget::itemSelectionChanged,
            this, [=]() {
                auto item = mp_categorySelector->selectedItems().at(0);
                if (!item) return;
                auto category = item->data(Qt::UserRole).toString();
                mp_contentManager->setCurrentCategoryFilter(category);
    });
}
