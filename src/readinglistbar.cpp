#include "readinglistbar.h"
#include "ui_readinglistbar.h"
#include "kiwixapp.h"
#include "zim/error.h"
#include "zim/item.h"

#include <QListWidgetItem>

ReadingListBar::ReadingListBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::readinglistbar)
{
    ui->setupUi(this);
    connect(KiwixApp::instance()->getLibrary(), &Library::bookmarksChanged,
            this, &ReadingListBar::setupList);
    connect(ui->listWidget, &QListWidget::itemActivated,
            this, &ReadingListBar::on_itemActivated);
    setupList();

    ui->label->setText(gt("reading-list-title"));
}

ReadingListBar::~ReadingListBar()
{
    delete ui;
}


void ReadingListBar::setupList()
{
    auto library = KiwixApp::instance()->getLibrary();
    auto bookmarks = library->getBookmarks();
    auto listWidget = ui->listWidget;
    listWidget->clear();
    for(auto& bookmark:bookmarks) {
        std::shared_ptr<zim::Archive> archive;
        try {
            archive = library->getArchive(QString::fromStdString(bookmark.getBookId()));
        } catch (std::out_of_range& e) {
            continue;
        }
        try {
            auto illustration = archive->getIllustrationItem(48);
            std::string content = illustration.getData();
            std::string mimeType = illustration.getMimetype();
            QPixmap pixmap;
            pixmap.loadFromData(reinterpret_cast<const uchar*>(content.data()), content.size());
            auto icon = QIcon(pixmap);
            auto item = new QListWidgetItem(
                icon,
                QString::fromStdString(bookmark.getTitle()),
                listWidget);
            item->setTextAlignment(Qt::TextWordWrap);
        } catch (zim::EntryNotFound& e) {
            auto item = new QListWidgetItem(
                QString::fromStdString(bookmark.getTitle()),
                listWidget);
            item->setTextAlignment(Qt::TextWordWrap);
        }
    }
}

void ReadingListBar::on_itemActivated(QListWidgetItem* item)
{
    int index = ui->listWidget->row(item);
    auto bookmark = KiwixApp::instance()->getLibrary()->getBookmarks(true).at(index);
    QUrl url;
    url.setScheme("zim");
    url.setHost(QString::fromStdString(bookmark.getBookId())+".zim");
    url.setPath(QString::fromStdString(bookmark.getUrl()));
    KiwixApp::instance()->openUrl(url);
}
