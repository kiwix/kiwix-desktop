#ifndef CONTENTMANAGERVIEW_H
#define CONTENTMANAGERVIEW_H

#include <QWidget>
#include "ui_contentmanagerview.h"

namespace Ui {
class contentmanagerview;
}

class ContentManagerView : public QWidget
{
    Q_OBJECT

public:
    explicit ContentManagerView(QWidget *parent = nullptr);
    ~ContentManagerView();
    QTreeView* getView() { return mp_ui->m_view; }
    QLineEdit* &getSearcher() { return mp_ui->searcher; }

private:
    Ui::contentmanagerview *mp_ui;
};

#endif // CONTENTMANAGERVIEW_H
