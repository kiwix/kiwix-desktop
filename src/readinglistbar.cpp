#include "readinglistbar.h"
#include "ui_readinglistbar.h"
#include "kiwixapp.h"
#include "zim/error.h"
#include "zim/item.h"

#include <QListWidgetItem>
#include <QFileDialog>

ReadingListBar::ReadingListBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::readinglistbar)
{
    ui->setupUi(this);
    connect(KiwixApp::instance()->getLibrary(), &Library::bookmarksChanged,
            this, &ReadingListBar::setupList);
    connect(ui->listWidget, &QListWidget::itemClicked,
            this, &ReadingListBar::onItemClicked);
    connect(ui->listWidget, &QListWidget::itemDoubleClicked,
            this, &ReadingListBar::onItemDoubleClicked);
    connect(ui->listWidget, &QListWidget::itemPressed, 
            this, [this](QListWidgetItem* item) {
                onItemPressed(item, QGuiApplication::mouseButtons());
            });
    connect(ui->listWidget, &QListWidget::itemActivated, 
            this, [this](QListWidgetItem* item) {
                onItemActivated(item, QGuiApplication::mouseButtons());
            });

    setupList();

    auto app = KiwixApp::instance();
    auto exportAction = app->getAction(KiwixApp::ExportReadingListAction);
    auto importAction = app->getAction(KiwixApp::ImportReadingListAction);
    connect(exportAction, &QAction::triggered, this, &ReadingListBar::onExport);
    connect(importAction, &QAction::triggered, this, &ReadingListBar::onImport);
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

// Receives single clicks
void ReadingListBar::onItemClicked(QListWidgetItem* item)
{
    clickKind = 1;
    QTimer::singleShot(QApplication::doubleClickInterval(), [=]() { // Give time for a double click be handled
        if (clickKind == 1) {
            openUrl(item, false);
        }
    });
}

// Receives double clicks
void ReadingListBar::onItemDoubleClicked(QListWidgetItem* item)
{ 
    clickKind = 2;
    openUrl(item, true);
}

// Receives single and middle click
void ReadingListBar::onItemPressed(QListWidgetItem* item, Qt::MouseButtons buttons)
{
    if (buttons & Qt::MiddleButton) {
        openUrl(item, true);
    }
}

// Receives left clicks and activation key
void ReadingListBar::onItemActivated(QListWidgetItem* item, Qt::MouseButtons buttons)
{
    if (!buttons) { // clicks are handled elsewhere, handle only the activation key case 
        openUrl(item, true);
    }
}

void ReadingListBar::onExport()
{
    auto app = KiwixApp::instance();
    auto kiwixLibrary = app->getLibrary()->getKiwixLibrary();
    QString fileName = QFileDialog::getSaveFileName(app->getMainWindow(),
                                                    gt("save-file-as-window-title"),
                                                    "kiwix_readinglist.xml", "(*.xml)");
    if (fileName.isEmpty())
        return;

    if (!kiwixLibrary->writeBookmarksToFile(fileName.toStdString()))
        app->showMessage(gt("export-reading-list-error"), gt("error-title"), QMessageBox::Information);
}

void ReadingListBar::onImport()
{
    auto app = KiwixApp::instance();
    auto library = app->getLibrary();
    QString fileName = QFileDialog::getOpenFileName(app->getMainWindow(),
                                                    gt("open-file"),
                                                    QString(), "(*.xml)");
    if (fileName.isEmpty())
        return;

    if (!library->readBookMarksFile(fileName.toStdString()))
        app->showMessage(gt("import-reading-list-error"), gt("error-title"), QMessageBox::Information);
}

void ReadingListBar::openUrl(QListWidgetItem* item, bool newTab)
{
    int index = ui->listWidget->row(item);
    auto bookmark = KiwixApp::instance()->getLibrary()->getBookmarks(true).at(index);
    QUrl url;
    url.setScheme("zim");
    url.setHost(QString::fromStdString(bookmark.getBookId())+".zim");
    url.setPath(QString::fromStdString(bookmark.getUrl()));
    KiwixApp::instance()->openUrl(url, newTab);
}
