#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"
#include "kiwixapp.h"
#include "kiwixchoicebox.h"

#include <QLocale>
#include <QDebug>

#include "klistwidgetitem.h"

ContentManagerSide::ContentManagerSide(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::contentmanagerside)
{
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    mp_ui->setupUi(this);
    QFile file(QString::fromUtf8(":/css/contentmanagerside.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    this->setStyleSheet(styleSheet);

    mp_ui->buttonGroup->setId(mp_ui->allFileButton, CatalogButtonId::ALL);
    mp_ui->buttonGroup->setId(mp_ui->localFileButton, CatalogButtonId::LOCAL);
    connect(mp_ui->buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *btn) {
        const auto id = mp_ui->buttonGroup->id(btn);
        mp_contentManager->setLocal(id == CatalogButtonId::LOCAL);
    });

    mp_ui->contentTypeButton->setIcon(QIcon(":/icons/caret-right-solid.svg"));
    mp_ui->contentTypeButton->setIconSize(QSize(12, 12));
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->allFileButton->setStyleSheet(
                    checked ? "*{font-weight: bold}" : "");});
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->localFileButton->setStyleSheet(
                    checked ?"*{font-weight: bold}" : "");});

    mp_ui->allFileButton->setText(gt("online-files"));
    mp_ui->localFileButton ->setText(gt("local-files"));
    mp_ui->contentTypeButton->setText(gt("content-type"));

    mp_categories = mp_ui->categories;
    mp_categories->setType(gt("category"));

    mp_languages = mp_ui->languages;
    mp_languages->setType(gt("language"));

    mp_contentTypeButton = mp_ui->contentTypeButton;


    connect(mp_contentTypeButton, &QCheckBox::toggled, this, [=](bool checked) {
        mp_ui->contentTypeSelector->setHidden(!checked);
    });
    mp_ui->contentTypeSelector->setHidden(true);

    mp_ui->contentTypeAllButton->setText(gt("all"));
    mp_ui->contentTypeAllButton->setStyleSheet("*{font-weight: 500;}");
    connect(mp_ui->contentTypeAllButton, &QCheckBox::clicked, this, [=](bool checked) {
        Q_UNUSED(checked);
        mp_ui->contentTypeAllButton->setStyleSheet("*{font-weight: 500;}");
        for (auto &contentTypeFilter : m_contentTypeFilters) {
            contentTypeFilter->setCheckState(Qt::Unchecked);
        }
        mp_contentManager->setCurrentContentTypeFilter(m_contentTypeFilters);
    });

    auto searcher = mp_ui->searcher;
    searcher->setPlaceholderText(gt("search-files"));
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
            mp_ui->contentTypeAllButton->setStyleSheet(activeFilter ? " * {color : #666666;} " : "*{font-weight: 500; color: black;}");
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
    const auto isLocal = mp_contentManager->isLocal();
    const auto checkedButton = mp_ui->buttonGroup->button(isLocal == CatalogButtonId::LOCAL);
    checkedButton->setChecked(true);
    checkedButton->setStyleSheet("*{font-weight: bold}");
    connect(mp_languages, &KiwixChoiceBox::choiceUpdated,
            this, [=](QStringList values) {
                if (values[0] == "all") {
                    mp_contentManager->setCurrentLanguage("*");
                    return;
                }
                mp_contentManager->setCurrentLanguage(values.join(","));
    });
    connect(mp_categories, &KiwixChoiceBox::choiceUpdated, this, [=](QStringList values) {
        mp_contentManager->setCurrentCategoryFilter(values.join(","));
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
    mp_categories->setSelections(categories, gt("all"));
}

void ContentManagerSide::setLanguages(ContentManager::LanguageList langList)
{
    mp_languages->setSelections(langList, QLocale::languageToString(QLocale().language()));
}
