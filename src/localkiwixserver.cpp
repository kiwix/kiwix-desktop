#include "localkiwixserver.h"
#include "ui_localkiwixserver.h"
#include "kiwixapp.h"
#include <kiwix/tools.h>
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
    connect(ui->closeButton, &QPushButton::clicked, this, &LocalKiwixServer::close);
    connect(ui->PortChooser, &QLineEdit::textChanged, ui->PortChooser, [=](const QString &text) {
        if (text.toInt() > 65535) {
            QString validText = text;
            validText.chop(1);
            ui->PortChooser->setText(validText);
        } else if (text.toInt() < 1) {
            QString defaultPort = QString::number(KiwixApp::instance()->getSettingsManager()->getKiwixServerPort());
            ui->PortChooser->setText(defaultPort);
        }
    });

    const auto interfacesMap = kiwix::getNetworkInterfacesIPv4Or6();
    QVector<QString> interfaces;
    interfaces.reserve(interfacesMap.size() + 1);
    for (const auto &interfacePair : interfacesMap) {
        QString ipv4 = QString::fromStdString(interfacePair.second.addr);
        if (!ipv4.isEmpty() && !ipv4.startsWith("169.254")) interfaces.push_back(ipv4);
        QString ipv6 = QString::fromStdString(interfacePair.second.addr6);
        if (!ipv6.isEmpty()) interfaces.push_back(ipv6);
    }
    std::sort(interfaces.begin(), interfaces.end());
    interfaces.push_front(QString(gt("ipv6")));
    interfaces.push_front(QString(gt("ipv4")));
    interfaces.push_front(QString(gt("all_ips")));
    for (const auto &interface : interfaces) {
        ui->IpChooser->addItem(interface);
    }
    QString ipAddress = KiwixApp::instance()->getSettingsManager()->getKiwixServerIpAddress();
    if (ipAddress == "0.0.0.0") ipAddress = "ipv4";
    ui->IpChooser->setCurrentText(ipAddress == "all_ips" || ipAddress == "ipv4" || ipAddress == "ipv6" ? gt(ipAddress) : ipAddress);
    ui->PortChooser->setText(QString::number(m_port));
    ui->PortChooser->setValidator(new QIntValidator(1, 65535, this));
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
    url.setUrl(m_url);
    QDesktopServices::openUrl(url);
}

void LocalKiwixServer::runOrStopServer()
{
    if (!m_active) {
        auto settingsManager = KiwixApp::instance()->getSettingsManager();
        m_port = ui->PortChooser->text().toInt();
        mp_server->setPort(m_port);
        settingsManager->setKiwixServerPort(m_port);
        mp_server->setIpMode(kiwix::IpMode::AUTO);
        mp_server->setAddress("");
        if (ui->IpChooser->currentText() == gt("all_ips")) {
            mp_server->setIpMode(kiwix::IpMode::ALL);
            settingsManager->setKiwixServerIpAddress("all_ips");
        } else if (ui->IpChooser->currentText() == gt("ipv4")) {
            mp_server->setIpMode(kiwix::IpMode::IPV4);
            settingsManager->setKiwixServerIpAddress("ipv4");
        } else if (ui->IpChooser->currentText() == gt("ipv6")) {
            mp_server->setIpMode(kiwix::IpMode::IPV6);
            settingsManager->setKiwixServerIpAddress("ipv6");
        } else {
            mp_server->setAddress(ui->IpChooser->currentText().toStdString());
            settingsManager->setKiwixServerIpAddress(ui->IpChooser->currentText());
        }
        if (!mp_server->start()) {
            QMessageBox messageBox;
            messageBox.critical(0,gt("error-title"),gt("error-launch-server-message"));
            return;
        }
        // gets first address only [one address is guaranteed to be there]
        m_url = QString::fromStdString(mp_server->getServerAccessUrls()[0]);
        ui->IpAddress->setText(m_url);
        ui->IpAddress->setReadOnly(true);
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
