#include "kiwixapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWebEngineProfile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->webview->page()->setUrl(QUrl("http://foo.zim"));
    //ui->webview->page()->setUrl(QUrl("http://localhost:8080"));

    QObject::connect(ui->webview, SIGNAL(urlChanged(const QUrl&)), this, SLOT(on_urlChanged_triggered(const QUrl&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->webview->reload();
}


void MainWindow::on_urlChanged_triggered(const QUrl& url)
{
    std::cout << "new url : " << url.toString().toUtf8().constData() << std::endl;
    ui->addressBar->setText(url.toString());
}
