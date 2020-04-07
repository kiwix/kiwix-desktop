#include "readinglistbar.h"
#include "ui_readinglistbar.h"
#include "kiwixapp.h"

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
        auto reader = library->getReader(QString::fromStdString(bookmark.getBookId()));
        if (reader == nullptr)
            continue;
        std::string content;
        std::string mimeType;
        reader->getFavicon(content, mimeType);
        QPixmap pixmap;
        pixmap.loadFromData(reinterpret_cast<const uchar*>(content.data()), content.size());
        auto icon = QIcon(pixmap);
        auto item = new QListWidgetItem(
            icon,
            QString::fromStdString(bookmark.getTitle()),
            listWidget);
        item->setTextAlignment(Qt::TextWordWrap);
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
