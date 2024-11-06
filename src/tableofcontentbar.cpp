#include "tableofcontentbar.h"
#include "ui_tableofcontentbar.h"
#include "kiwixapp.h"
#include <QJsonObject>
#include <QTreeWidgetItem>

TableOfContentBar::TableOfContentBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::tableofcontentbar)
{
    ui->setupUi(this);
    ui->titleLabel->setText(gt("table-of-content"));
    ui->tree->setRootIsDecorated(false);
    ui->tree->setItemsExpandable(false);
    connect(ui->tree, &QTreeWidget::itemClicked, this, &TableOfContentBar::onTreeItemActivated);
    connect(ui->tree, &QTreeWidget::itemActivated, this, &TableOfContentBar::onTreeItemActivated);
}

TableOfContentBar::~TableOfContentBar()
{
    delete ui;
}

void TableOfContentBar::onTreeItemActivated(QTreeWidgetItem *item)
{
    emit navigationRequested(m_url, item->data(0, Qt::UserRole).toString());
}

namespace
{

QTreeWidgetItem* createChildItem(QTreeWidgetItem* parent, const QJsonObject& headerObj)
{
    const auto item = new QTreeWidgetItem(parent);
    item->setExpanded(true);
    item->setData(0, Qt::DisplayRole, headerObj["text"].toString());
    item->setData(0, Qt::UserRole, headerObj["anchor"].toString());

    return item;
}

QJsonArray takeDeeperEntries(QJsonArray& headerArr, int level)
{
    QJsonArray result;
    while (!headerArr.isEmpty())
    {
        const auto& nextHeader = headerArr.first().toObject();
        if (nextHeader["level"].toInt() <= level)
           break;

        result.push_back(nextHeader);
        headerArr.pop_front();
    }
    return result;
}

void createSubTree(QTreeWidgetItem* parent, QJsonArray& headerArr)
{
    while (!headerArr.isEmpty())
    {
        const auto childHeader = headerArr.takeAt(0).toObject();
        const int childLevel = childHeader["level"].toInt();
        QTreeWidgetItem* childItem = createChildItem(parent, childHeader);
        QJsonArray deeperEntries = takeDeeperEntries(headerArr, childLevel);
        createSubTree(childItem, deeperEntries);
    }
}

}

void TableOfContentBar::setupTree(const QJsonObject& headers)
{
    const auto headerUrl = headers["url"].toString();
    const auto webView = KiwixApp::instance()->getTabWidget()->currentWebView();
    const auto currentUrl = webView->url().url(QUrl::RemoveFragment);
    if (headerUrl != currentUrl)
        return;
    
    m_url = headerUrl;
    ui->tree->clear();
    QJsonArray headerArr = headers["headers"].toArray();
    createSubTree(ui->tree->invisibleRootItem(), headerArr);
}
