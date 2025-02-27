#include "tableofcontentbar.h"
#include "ui_tableofcontentbar.h"
#include "kiwixapp.h"
#include <QJsonObject>
#include <QTreeWidgetItem>
#include <QTimer>

TableOfContentBar::TableOfContentBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::tableofcontentbar)
{
    ui->setupUi(this);
    ui->titleLabel->setFont(QFont("Selawik", 18, QFont::Weight::Medium));
    ui->titleLabel->setText(gt("table-of-content"));
    ui->hideLabel->setFont(QFont("Selawik", 12));
    ui->hideLabel->setTextFormat(Qt::RichText);

    /* href is needed to make hide clickable, but not used. So Kiwix it is :) */
    ui->hideLabel->setText("<a href=\"https://kiwix.org/en/\">" + gt("hide") + "</a>");
    connect(ui->hideLabel, &QLabel::linkActivated, this, [=](){
        KiwixApp::instance()->getAction(KiwixApp::ToggleTOCAction)->setChecked(false);
    });

    ui->tree->setRootIsDecorated(false);
    ui->tree->setItemsExpandable(false);
    connect(ui->tree, &QTreeWidget::itemClicked, this, &TableOfContentBar::onTreeItemActivated);
    connect(ui->tree, &QTreeWidget::itemActivated, this, &TableOfContentBar::onTreeItemActivated);

    // Setup debounce timer
    m_clickDebounceTimer.setSingleShot(true);
    m_clickDebounceTimer.setInterval(300); // 300ms debounce
}

TableOfContentBar::~TableOfContentBar()
{
    delete ui;
}

void TableOfContentBar::onTreeItemActivated(QTreeWidgetItem *item)
{
    //Safety check
    if (!item) {
        return;
    }

    // Get the anchor from the item
    QVariant anchorVariant = item->data(0, Qt::UserRole);
    if (!anchorVariant.isValid()) {
        return;
    }

    QString anchor = anchorVariant.toString();
    if (anchor.isEmpty() || m_url.isEmpty()) {
        return;
    }

    if (m_isNavigating || (anchor == m_lastAnchor && m_clickDebounceTimer.isActive())) {
        return;
    }

    m_isNavigating = true;
    m_lastAnchor = anchor;
    m_clickDebounceTimer.start();

    QTimer::singleShot(10, this, [this, anchor]() {
        emit navigationRequested(m_url, anchor);

        QTimer::singleShot(300, this, [this]() {
            m_isNavigating = false;
        });
    });
}

namespace
{

QTreeWidgetItem* createChildItem(QTreeWidgetItem* parent, const QString& childNo, const QJsonObject& headerObj)
{
    const auto item = new QTreeWidgetItem(parent);
    item->setExpanded(true);

    const auto display = childNo + "  " + headerObj["text"].toString();
    item->setToolTip(0, display);
    item->setData(0, Qt::DisplayRole, display);
    item->setData(0, Qt::FontRole, QFont("Selawik", 12));
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

void createSubTree(QTreeWidgetItem* parent, QString parentNo, QJsonArray& headerArr)
{
    while (!headerArr.isEmpty())
    {
        const auto childHeader = headerArr.takeAt(0).toObject();
        const int childLevel = childHeader["level"].toInt();
        const QString childNo = parentNo + QString::number(parent->childCount() + 1);
        QTreeWidgetItem* childItem = createChildItem(parent, childNo, childHeader);
        QJsonArray deeperEntries = takeDeeperEntries(headerArr, childLevel);
        createSubTree(childItem, childNo + ".", deeperEntries);
    }
}

}

void TableOfContentBar::setupTree(const QJsonObject& headers)
{
    const auto webView = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!webView)
        return;

    const auto headerUrl = headers["url"].toString();
    const auto currentUrl = webView->url().url(QUrl::RemoveFragment);
    if (headerUrl != currentUrl)
        return;

    m_url = headerUrl;
    ui->tree->clear();
    QJsonArray headerArr = headers["headers"].toArray();
    createSubTree(ui->tree->invisibleRootItem(), "", headerArr);

    // Update selection based on current URL fragment
    updateSelectionFromFragment(webView->url().fragment());
}

void TableOfContentBar::updateSelectionFromFragment(const QString& fragment)
{
    if (fragment.isEmpty() || !ui || !ui->tree) {
        return;
    }

    // Find the item with the matching anchor
    QTreeWidgetItemIterator it(ui->tree);
    while (*it) {
        QVariant anchorVariant = (*it)->data(0, Qt::UserRole);
        if (!anchorVariant.isValid()) {
            ++it;
            continue;
        }

        QString anchor = anchorVariant.toString();
        if (anchor == fragment) {
            // Select the item without triggering navigation
            ui->tree->blockSignals(true);
            ui->tree->setCurrentItem(*it);
            ui->tree->scrollToItem(*it);
            ui->tree->blockSignals(false);
            break;
        }
        ++it;
    }
}