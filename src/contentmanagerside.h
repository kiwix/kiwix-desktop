#ifndef CONTENTMANAGERSIDE_H
#define CONTENTMANAGERSIDE_H

#include <QWidget>
#include "contentmanager.h"

namespace Ui {
class contentmanagerside;
}

class ContentManagerSide : public QWidget
{
    Q_OBJECT

public:
    explicit ContentManagerSide(QWidget *parent = 0);
    ~ContentManagerSide();

    void setContentManager(ContentManager* contentManager) { mp_contentManager = contentManager; }

private:
    Ui::contentmanagerside *mp_ui;
    ContentManager* mp_contentManager;
};

#endif // CONTENTMANAGERSIDE_H
