#ifndef CONTENTMANAGERSIDE_H
#define CONTENTMANAGERSIDE_H

#include "contentmanager.h"
#include "contenttypefilter.h"

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>

namespace Ui {
class contentmanagerside;
}

class ContentManagerSide : public QWidget
{
    Q_OBJECT

public:
    explicit ContentManagerSide(QWidget *parent = 0);
    ~ContentManagerSide();

    void setContentManager(ContentManager* contentManager);

private:
    Ui::contentmanagerside *mp_ui;
    ContentManager* mp_contentManager;
    QCheckBox* mp_languageButton;
    QListWidget* mp_languageSelector;
    QCheckBox* mp_categoryButton;
    QListWidget* mp_categorySelector;
    QCheckBox* mp_contentTypeButton;
    QList<ContentTypeFilter*> m_contentTypeFilters;
};

#endif // CONTENTMANAGERSIDE_H
