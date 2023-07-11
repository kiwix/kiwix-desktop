#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"
#include "kiwixapp.h"

#include <QLocale>
#include <QDebug>

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
    mp_ui->contentTypeButton->setText(gt("content-type"));

    mp_languageButton = mp_ui->languageButton;
    mp_languageSelector = mp_ui->languageSelector;
    connect(mp_languageButton, &QCheckBox::toggled, this, [=](bool checked) { mp_languageSelector->setHidden(!checked); });
    mp_languageSelector->setHidden(true);

    mp_categoryButton = mp_ui->categoryButton;
    mp_categorySelector = mp_ui->categorySelector;
    connect(mp_categoryButton, &QCheckBox::toggled, this, [=](bool checked) { mp_categorySelector->setHidden(!checked); });
    mp_categorySelector->setHidden(true);

    mp_contentTypeButton = mp_ui->contentTypeButton;
    connect(mp_contentTypeButton, &QCheckBox::toggled, this, [=](bool checked) { mp_ui->contentTypeSelector->setHidden(!checked); });
    mp_ui->contentTypeSelector->setHidden(true);

    mp_ui->contentTypeAllButton->setText(gt("all"));
    mp_ui->contentTypeAllButton->setStyleSheet("*{font-weight: bold}");
    connect(mp_ui->contentTypeAllButton, &QCheckBox::clicked, this, [=](bool checked) {
        Q_UNUSED(checked);
        mp_ui->contentTypeAllButton->setStyleSheet("*{font-weight: bold}");
        for (auto &contentTypeFilter : m_contentTypeFilters) {
            contentTypeFilter->setCheckState(Qt::Unchecked);
        }
        mp_contentManager->setCurrentContentTypeFilter(m_contentTypeFilters);
    });

    auto searcher = mp_ui->searcher;
    searcher->setPlaceholderText(gt("search-files"));
    QFile file(QString::fromUtf8(":/css/_contentManager.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    searcher->setStyleSheet(styleSheet);
    QIcon searchIcon = QIcon(":/icons/search.svg");
    searcher->addAction(searchIcon, QLineEdit::LeadingPosition);
    connect(searcher, &QLineEdit::textChanged, [searcher](){
        KiwixApp::instance()->getContentManager()->setSearch(searcher->text());
    });
    
    ContentTypeFilter* videosFilter = new ContentTypeFilter("pictures", this);
    ContentTypeFilter* picturesFilter = new ContentTypeFilter("videos", this);
    ContentTypeFilter* detailsFilter = new ContentTypeFilter("details", this);
    m_contentTypeFilters.push_back(videosFilter);
    m_contentTypeFilters.push_back(picturesFilter);
    m_contentTypeFilters.push_back(detailsFilter);

    auto layout = static_cast<QVBoxLayout*>(mp_ui->contentTypeSelector->layout());
    for (auto &contentTypeFilter : m_contentTypeFilters) {
        layout->addWidget(contentTypeFilter, 0, Qt::AlignTop);
        connect(contentTypeFilter, &QCheckBox::clicked, this, [=](bool checked) {
            Q_UNUSED(checked);
            bool activeFilter = false;
            for (auto &contentTypeFilter : m_contentTypeFilters) {
                if (contentTypeFilter->checkState() != Qt::Unchecked) {
                    activeFilter = true;
                    break;
                }
            }
            mp_ui->contentTypeAllButton->setStyleSheet(activeFilter ? "" : "*{font-weight: bold}");
            mp_contentManager->setCurrentContentTypeFilter(m_contentTypeFilters);
        });
    }

    setCategories(KiwixApp::instance()->getContentManager()->getCategories());
    setLanguages(KiwixApp::instance()->getContentManager()->getLanguages());
    connect(KiwixApp::instance()->getContentManager(), &ContentManager::categoriesLoaded, this, &ContentManagerSide::setCategories);
    connect(KiwixApp::instance()->getContentManager(), &ContentManager::languagesLoaded, this, &ContentManagerSide::setLanguages);
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
                auto lang = item->data(Qt::UserRole).toString();
                if (lang == "all") {
                    mp_contentManager->setCurrentLanguage("*");
                    return;
                }
                mp_contentManager->setCurrentLanguage(lang);
    });
    connect(mp_categorySelector, &QListWidget::itemSelectionChanged,
            this, [=]() {
                auto item = mp_categorySelector->selectedItems().at(0);
                if (!item) return;
                auto category = item->data(Qt::UserRole).toString();
                mp_contentManager->setCurrentCategoryFilter(category);
    });
}

QString beautify(QString word)
{
    word = word.replace("_", " ");
    word[0] = word[0].toUpper();
    return word;
}

void ContentManagerSide::setCategories(QStringList categories)
{
    mp_categorySelector->blockSignals(true);
    mp_categorySelector->setHidden(true);
    mp_categorySelector->clear();
    mp_categorySelector->blockSignals(false);
    for (auto category: categories)
    {
        auto item = new KListWidgetItem(beautify(category));
        item->setData(Qt::UserRole, category);
        mp_categorySelector->addItem(item);
        if (category ==  "all")
        {
            item->setSelected(true);
        }
    }
}

void ContentManagerSide::setLanguages(ContentManager::LanguageList langList)
{
    mp_languageSelector->blockSignals(true);
    mp_languageSelector->setHidden(true);
    mp_languageSelector->clear();
    mp_languageSelector->blockSignals(false);
    for(auto lang: langList)
    {
        auto currentLang = QLocale().language();
        auto item = new KListWidgetItem(lang.second);
        item->setData(Qt::UserRole, lang.first);
        mp_languageSelector->addItem(item);
        if (lang.second == QLocale::languageToString(currentLang)) {
            item->setSelected(true);
        }
    }
    mp_languageSelector->sortItems();
    auto item = new KListWidgetItem("All");
    item->setData(Qt::UserRole, "all");
    mp_languageSelector->insertItem(0, item);
}
