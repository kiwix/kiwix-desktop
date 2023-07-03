#ifndef CONTENTMANAGERVIEW_H
#define CONTENTMANAGERVIEW_H

#include <QWidget>
#include "ui_contentmanagerview.h"
#include "kiwixloader.h"

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

public slots:
    void showLoader(bool show);

private:
    Ui::contentmanagerview *mp_ui;
    KiwixLoader *loader;
};

#endif // CONTENTMANAGERVIEW_H
