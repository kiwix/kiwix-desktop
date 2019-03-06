#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"

#include <QLocale>

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
    mp_languageButton = mp_ui->languageButton;
    mp_languageSelector = mp_ui->languageSelector;
    connect(mp_languageButton, &QCheckBox::toggled, this, [=](bool checked) { mp_languageSelector->setHidden(!checked); });
    mp_languageSelector->setHidden(true);


    for (auto lang:
        {
         QLocale::Afar,
         QLocale::Afrikaans,
         QLocale::Akan,
         QLocale::Amharic,
         QLocale::Arabic,
         QLocale::Assamese,
         QLocale::Azerbaijani,
         QLocale::Bashkir,
         QLocale::Belarusian,
         QLocale::Bulgarian,
         QLocale::Bambara,
         QLocale::Bengali,
         QLocale::Tibetan,
         QLocale::Breton,
         QLocale::Bosnian,
         QLocale::Catalan,
         QLocale::Chechen,
         QLocale::Corsican,
         QLocale::Czech,
         QLocale::Church,
         QLocale::Chuvash,
         QLocale::Welsh,
         QLocale::Danish,
         QLocale::German,
         QLocale::Divehi,
         QLocale::Dzongkha,
         QLocale::Ewe,
         QLocale::Greek,
         QLocale::English,
         QLocale::Spanish,
         QLocale::Estonian,
         QLocale::Basque,
         QLocale::Persian,
         QLocale::Fulah,
         QLocale::Finnish,
         QLocale::Faroese,
         QLocale::French,
         QLocale::WesternFrisian,
         QLocale::Irish,
         QLocale::Gaelic,
         QLocale::Galician,
         QLocale::Guarani,
         QLocale::Gujarati,
         QLocale::Manx,
         QLocale::Hausa,
         QLocale::Hebrew,
         QLocale::Hindi,
         QLocale::Croatian,
         QLocale::Hungarian,
         QLocale::Armenian,
         QLocale::Interlingua,
         QLocale::Indonesian,
         QLocale::Igbo,
         QLocale::Icelandic,
         QLocale::Italian,
         QLocale::Inuktitut,
         QLocale::Japanese,
         QLocale::Javanese,
         QLocale::Georgian,
         QLocale::Kikuyu,
         QLocale::Kazakh,
         QLocale::Greenlandic,
         QLocale::Khmer,
         QLocale::Kannada,
         QLocale::Korean,
         QLocale::Kashmiri,
         QLocale::Kurdish,
         QLocale::Cornish,
         QLocale::Kirghiz,
         QLocale::Luxembourgish,
         QLocale::Ganda,
         QLocale::Lingala,
         QLocale::Lao,
         QLocale::Lithuanian,
         QLocale::Latvian,
         QLocale::Malagasy,
         QLocale::Maori,
         QLocale::Maori,
         QLocale::Macedonian,
         QLocale::Malayalam,
         QLocale::Mongolian,
         QLocale::Marathi,
         QLocale::Malay,
         QLocale::Maltese,
         QLocale::Burmese,
         QLocale::Nepali,
         QLocale::Dutch,
         QLocale::NorwegianNynorsk,
         QLocale::NorwegianBokmal,
         QLocale::Nyanja,
         QLocale::Occitan,
         QLocale::Oromo,
         QLocale::Oriya,
         QLocale::Ossetic,
         QLocale::Punjabi,
         QLocale::Polish,
         QLocale::Pashto,
         QLocale::Portuguese,
         QLocale::Quechua,
         QLocale::Romansh,
         QLocale::Rundi,
         QLocale::Romanian,
         QLocale::Russian,
         QLocale::Kinyarwanda,
         QLocale::Sanskrit,
         QLocale::Sindhi,
         QLocale::NorthernSami,
         QLocale::Sango,
         QLocale::Sinhala,
         QLocale::Slovak,
         QLocale::Slovenian,
         QLocale::Shona,
         QLocale::Somali,
         QLocale::Albanian,
         QLocale::Serbian,
         QLocale::Swati,
         QLocale::SouthernSotho,
         QLocale::Swedish,
         QLocale::Swahili,
         QLocale::Tamil,
         QLocale::Telugu,
         QLocale::Tajik,
         QLocale::Thai,
         QLocale::Tigrinya,
         QLocale::Turkmen,
         QLocale::Filipino,
         QLocale::Tswana,
         QLocale::Tongan,
         QLocale::Turkish,
         QLocale::Tsonga,
         QLocale::Tatar,
         QLocale::Uighur,
         QLocale::Ukrainian,
         QLocale::Urdu,
         QLocale::Uzbek,
         QLocale::Venda,
         QLocale::Vietnamese,
         QLocale::Walloon,
         QLocale::Wolof,
         QLocale::Xhosa,
         QLocale::Yoruba,
         QLocale::Chinese,
         QLocale::Zulu,
    })
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
        auto item = new QListWidgetItem(QLocale::languageToString(locale.language()));
        item->setData(Qt::UserRole, lang);
        mp_languageSelector->addItem(item);
        if (lang == currentLang) {
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
                auto lang = QLocale::Language(item->data(Qt::UserRole).toInt());
                auto locale = QLocale(QLocale::Language(item->data(Qt::UserRole).toInt()));
                mp_contentManager->setCurrentLanguage(locale.name().split("_").at(0));});
}
