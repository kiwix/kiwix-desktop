#include "readinglistbar.h"
#include "ui_readinglistbar.h"
#include "kiwixapp.h"
#include "zim/error.h"
#include "zim/item.h"

#include <QListWidgetItem>
#include <QFileDialog>
#include <QStandardPaths>

const QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

ReadingListBar::ReadingListBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::readinglistbar)
{
    ui->setupUi(this);
    ui->titleLabel->setText(gt("reading-list"));

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

    QMenu *portMenu = new QMenu(this);
    portMenu->addAction(exportAction);
    portMenu->addAction(importAction);
    ui->readingListMenuButton->setMenu(portMenu);
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
        const auto zimId = QString::fromStdString(bookmark.getBookId());
        try {
            library->getArchive(zimId);
        } catch (std::out_of_range& e) {
            continue;
        }
        new QListWidgetItem(
            library->getBookIcon(zimId),
            QString::fromStdString(bookmark.getTitle()),
            listWidget);
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
    auto suggestedFilePath = documentsDir + "/kiwix_readinglist.xml";
    QString fileName = QFileDialog::getSaveFileName(app->getMainWindow(),
                                                    gt("save-file-as-window-title"),
                                                    suggestedFilePath, "(*.xml)");
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
                                                    documentsDir, "(*.xml)");
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
