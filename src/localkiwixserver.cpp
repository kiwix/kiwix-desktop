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
    connect(ui->PortChooser, &QLineEdit::textChanged, ui->PortChooser, [=](const QString &text){
        if(text.toInt() > 65535 || text.toInt() < 1) {
            QString validText = text;
            validText.chop(1);
            ui->PortChooser->setText(validText);
        }
    });

    const auto interfacesMap = kiwix::getNetworkInterfaces();
    QVector<QString> interfaces;
    interfaces.reserve(interfacesMap.size() + 1);
    for (const auto &interfacePair : interfacesMap) {
        QString ip = QString::fromStdString(interfacePair.second);
        interfaces.push_back(ip);
    }
    interfaces.push_back(QString("0.0.0.0"));
    std::sort(interfaces.begin(), interfaces.end());
    for (const auto &interface : interfaces) {
        ui->IpChooser->addItem(interface);
    }
    ui->IpChooser->setCurrentText(KiwixApp::instance()->getSettingsManager()->getKiwixServerIp());
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
    url.setScheme("http");
    url.setHost(m_ipAddress);
    url.setPort(m_port);
    QDesktopServices::openUrl(url);
}

void LocalKiwixServer::runOrStopServer()
{
    if (!m_active) {
        auto settingsManager = KiwixApp::instance()->getSettingsManager();
        m_port = ui->PortChooser->text().toInt();
        if (m_port == 0) {
            m_port = settingsManager->getKiwixServerPort();
        }
        mp_server->setPort(m_port);
        m_ipAddress = ui->IpChooser->currentText();
        settingsManager->setKiwixServerPort(m_port);
        settingsManager->setKiwixServerIpAddress(m_ipAddress);
        m_ipAddress = (m_ipAddress != "0.0.0.0") ? ui->IpChooser->currentText() : QString::fromStdString(kiwix::getBestPublicIp());
        mp_server->setAddress(ui->IpChooser->currentText().toStdString());
        ui->IpAddress->setText("http://" + m_ipAddress + ":" + QString::number(m_port));
        ui->IpAddress->setReadOnly(true);
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
