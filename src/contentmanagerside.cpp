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

    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->allFileButton->setStyleSheet(
                    checked ? "*{font-weight: bold}" : "");});
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->localFileButton->setStyleSheet(
                    checked ?"*{font-weight: bold}" : "");});

    mp_ui->allFileButton->setText(gt("online-files"));
    mp_ui->localFileButton ->setText(gt("local-files"));

    mp_categories = mp_ui->categories;
    mp_categories->setType("category");

    mp_languages = mp_ui->languages;
    mp_languages->setType("language");

    mp_contentType = mp_ui->contentType;
    mp_contentType->setType("content-type");

    auto searcher = mp_ui->searcher;
    searcher->setPlaceholderText(gt("search-files"));
    QIcon searchIcon = QIcon(":/icons/search.svg");

    searcher->addAction(searchIcon, QLineEdit::LeadingPosition);
    connect(searcher, &QLineEdit::textChanged, [searcher](){
        KiwixApp::instance()->getContentManager()->setSearch(searcher->text());
    });

    QList<QPair<QString, QString>> contentTypeList = {
      {"_pictures:yes", gt("pictures")},
      {"_pictures:no", gt("no-pictures")},
      {"_videos:yes", gt("videos")},
      {"_videos:no", gt("no-videos")},
      {"_details:yes", gt("details")},
      {"_details:no", gt("no-details")}
    };

    mp_contentType->setSelections(contentTypeList, KiwixApp::instance()->getSettingsManager()->getContentType());

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
                    values = QStringList();
                }
                mp_contentManager->setCurrentLanguage(values);
    });
    connect(mp_categories, &KiwixChoiceBox::choiceUpdated, this, [=](QStringList values) {
        mp_contentManager->setCurrentCategoryFilter(values);
    });
    connect(mp_contentType, &KiwixChoiceBox::choiceUpdated, this, [=](QStringList values) {
       if (values[0] == "all") {
           values = QStringList();
       }
       mp_contentManager->setCurrentContentTypeFilter(values);
    });
}

void ContentManagerSide::setCategories(QStringList categories)
{
    mp_categories->setSelections(categories, KiwixApp::instance()->getSettingsManager()->getCategoryList());
}

void ContentManagerSide::setLanguages(ContentManager::LanguageList langList)
{
    mp_languages->setSelections(langList, KiwixApp::instance()->getSettingsManager()->getLanguageList());
}
