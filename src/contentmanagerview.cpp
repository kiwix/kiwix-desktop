#include "contentmanagerview.h"
#include <QFile>
#include "kiwixapp.h"
#include "contentmanagerdelegate.h"
#include <QLineEdit>
#include "ui_contentmanagerview.h"

ContentManagerView::ContentManagerView(QWidget *parent)
    : QWidget(parent), mp_ui(new Ui::contentmanagerview)
{
    mp_ui->setupUi(this);
    mp_ui->m_view->setSortingEnabled(true);
    mp_ui->m_view->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/_contentManager.css"));
    mp_ui->m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    auto managerDelegate = new ContentManagerDelegate();
    mp_ui->m_view->setItemDelegate(managerDelegate);
    mp_ui->m_view->setCursor(Qt::PointingHandCursor);

    loader = new KiwixLoader(mp_ui->loading);
    mp_ui->stackedWidget->setCurrentIndex(0);

    connect(mp_ui->m_view, &QTreeView::clicked, [=](QModelIndex index) {
        if (index.column() == (mp_ui->m_view->model()->columnCount() - 1))
            return;

        auto zeroColIndex = index.siblingAtColumn(0);
        if (mp_ui->m_view->isExpanded(zeroColIndex)) {
            mp_ui->m_view->collapse(zeroColIndex);
        } else {
            mp_ui->m_view->expand(zeroColIndex);
        }
    });
}

ContentManagerView::~ContentManagerView()
{

}

void ContentManagerView::showLoader(bool show)
{
    mp_ui->stackedWidget->setCurrentIndex(show);
    if (show) {
        loader->startAnimation();
    } else {
        loader->stopAnimation();
    }
}
