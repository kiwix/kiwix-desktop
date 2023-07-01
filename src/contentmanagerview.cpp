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
    QFile file(QString::fromUtf8(":/css/_contentManager.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    mp_ui->m_view->setStyleSheet(styleSheet);
    mp_ui->m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    auto managerDelegate = new ContentManagerDelegate();
    mp_ui->m_view->setItemDelegate(managerDelegate);
    mp_ui->m_view->setCursor(Qt::PointingHandCursor);

    auto searcher = mp_ui->searcher;
    searcher->setPlaceholderText(gt("search-files"));
    searcher->setStyleSheet(styleSheet);

    QIcon searchIcon = QIcon(":/icons/search.svg");
    searcher->addAction(searchIcon, QLineEdit::LeadingPosition);

    connect(searcher, &QLineEdit::textChanged, [searcher](){
        KiwixApp::instance()->getContentManager()->setSearch(searcher->text());
    });
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
