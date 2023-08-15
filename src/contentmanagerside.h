#ifndef CONTENTMANAGERSIDE_H
#define CONTENTMANAGERSIDE_H

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>
#include "contentmanager.h"
#include "contenttypefilter.h"

namespace Ui {
class contentmanagerside;
}

class KiwixChoiceBox;

class ContentManagerSide : public QWidget
{
    Q_OBJECT

public:
    enum CatalogButtonId {
        ALL = 0,
        LOCAL = 1
    };
    explicit ContentManagerSide(QWidget *parent = 0);
    ~ContentManagerSide();

    void setContentManager(ContentManager* contentManager);

private:
    Ui::contentmanagerside *mp_ui;
    ContentManager* mp_contentManager;
    KiwixChoiceBox *mp_categories;
    KiwixChoiceBox *mp_languages;
    KiwixChoiceBox *mp_contentType;
    QCheckBox* mp_contentTypeButton;
    QStringList m_contentTypeFilters;

public slots:
    void setCategories(QStringList);
    void setLanguages(ContentManager::LanguageList);
};

#endif // CONTENTMANAGERSIDE_H
