#include "localkiwixserver.h"
#include "ui_localkiwixserver.h"
#include "kiwixapp.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <thread>

LocalKiwixServer::LocalKiwixServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocalKiwixServer)
{
    ui->setupUi(this);

    QFile styleFile(":/css/localServer.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    setStyleSheet(style);

    mp_server = KiwixApp::instance()->getLocalServer();
    m_port = mp_server->getPort();

    connect(ui->KiwixServerButton, SIGNAL(clicked()), this, SLOT(runOrStopServer()));
    connect(ui->OpenInBrowserButton, SIGNAL(clicked()), this, SLOT(openInBrowser()));
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::portChanged,
            this, [=](int port) { m_port = port; });
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            m_ipAddress = address.toString();
            break;
        }
    }
    ui->OpenInBrowserButton->setVisible(false);
    ui->IpAddress->setVisible(false);
}

LocalKiwixServer::~LocalKiwixServer()
{
    delete ui;
}

void LocalKiwixServer::openInBrowser()
{
    QUrl url;
    url.setScheme("http");
    url.setHost(m_ipAddress);
    url.setPort(mp_server->getPort());
    QDesktopServices::openUrl(url);
}

void LocalKiwixServer::runOrStopServer()
{
    if (!m_active) {
        mp_server->setPort(m_port);
        mp_server->run();
        ui->IpAddress->setText(m_ipAddress + ":" + QString::number(m_port));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!mp_server->isRunning()) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","An error has occured !");
            return;
        }
        m_active = true;
    } else {
        mp_server->shutDown();
        m_active = false;
    }

    if (m_active) {
        ui->KiwixServerButton->setText("Stop Kiwix Server");
        QUrl url("qrc:/texts/kiwixServerRunningText.html");
        ui->KiwixServerText->setSource(url);
        ui->OpenInBrowserButton->setVisible(true);
        ui->IpAddress->setVisible(true);
    } else {
        ui->KiwixServerButton->setText("Start Kiwix Server");
        QUrl url("qrc:/texts/kiwixServerText.html");
        ui->KiwixServerText->setSource(url);
        ui->OpenInBrowserButton->setVisible(false);
        ui->IpAddress->setVisible(false);
    }
}