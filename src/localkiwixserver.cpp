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
    setWindowFlag(Qt::FramelessWindowHint, true);
    ui->setupUi(this);

    QFile styleFile(":/css/localServer.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    setStyleSheet(style);

    mp_server = KiwixApp::instance()->getLocalServer();
    m_port = KiwixApp::instance()->getSettingsManager()->getKiwixServerPort();

    connect(ui->KiwixServerButton, SIGNAL(clicked()), this, SLOT(runOrStopServer()));
    connect(ui->OpenInBrowserButton, SIGNAL(clicked()), this, SLOT(openInBrowser()));
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::portChanged,
            this, [=](int port) { m_port = port; });
    connect(ui->closeButton, &QPushButton::clicked, this, &LocalKiwixServer::close);

    const auto interfacesMap = mp_server->getNetworkInterfaces();
    for(auto interfacePair : interfacesMap) {
        QString ip = QString::fromStdString(interfacePair.second);
        ui->IpChooser->addItem(ip);
    }
    ui->IpChooser->addItem("0.0.0.0");
    ui->IpChooser->setCurrentText(QString::fromStdString(mp_server->getBestPublicIp())); // put best Public Ip as default
    ui->PortChooser->setText(QString::number(m_port));
    ui->KiwixServerButton->setStyleSheet("QPushButton {background-color: RoyalBlue;"
                                                      "color: white;"
                                                      "padding: 5px;"
                                                      "border-radius: 3px;}"
                                         "QPushButton:hover {background-color: DodgerBlue;}");
    ui->label->setText(gt("local-kiwix-server"));
    ui->KiwixServerText->setText(gt("kiwix-server-description"));
    ui->OpenInBrowserButton->setText(gt("open-in-browser"));
    ui->KiwixServerButton->setText(gt("start-kiwix-server"));
    ui->closeButton->setText(gt("close"));
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
    url.setPort(m_port);
    QDesktopServices::openUrl(url);
}

void LocalKiwixServer::runOrStopServer()
{
    if (!m_active) {
        m_port = ui->PortChooser->text().toInt();
        m_ipAddress = (ui->IpChooser->currentText() != "0.0.0.0") ? ui->IpChooser->currentText() : QString::fromStdString(mp_server->getBestPublicIp());
        mp_server->setPort(m_port);
        KiwixApp::instance()->getSettingsManager()->setKiwixServerPort(m_port);
        mp_server->setAddress(ui->IpChooser->currentText().toStdString());
        ui->IpAddress->setText("http://" + m_ipAddress + ":" + QString::number(m_port));
        if (!mp_server->start()) {
            QMessageBox messageBox;
            messageBox.critical(0,gt("error-title"),gt("error-launch-server-message"));
            return;
        }
        m_active = true;
    } else {
        mp_server->stop();
        m_active = false;
    }

    if (m_active) {
        ui->KiwixServerButton->setText(gt("stop-kiwix-server"));
        ui->KiwixServerText->setText(gt("kiwix-server-running-message"));
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->KiwixServerButton->setText(gt("start-kiwix-server"));
        ui->KiwixServerText->setText(gt("kiwix-server-description"));
        ui->stackedWidget->setCurrentIndex(0);
    }
}
