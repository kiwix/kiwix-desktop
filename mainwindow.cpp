
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kiwixapp.h"
#include "kconstants.h"

#include <QWebEngineProfile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->tabBar()->setExpanding(false);
#if !SYSTEMTITLEBAR
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayReader(std::shared_ptr<kiwix::Reader> reader)
{
    auto webview = new KiwixWebView();
    std::string favicon_content;
    std::string favicon_mimetype;
    reader->getFavicon(favicon_content, favicon_mimetype);
    QPixmap pixmap;
    pixmap.loadFromData((const uchar*)favicon_content.data(), favicon_content.size());
    auto icon = QIcon(pixmap);
    // Ownership of webview is passed to the tabWidget
    ui->tabWidget->addTab(webview, icon, QString::fromStdString(reader->getTitle()));
    webview->initFromReader(reader);
}
