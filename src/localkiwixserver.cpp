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

    const auto interfacesMap = kiwix::getNetworkInterfaces();
    QVector<QString> interfaces;
    interfaces.reserve(interfacesMap.size() + 1);
    for (const auto &interfacePair : interfacesMap) {
        QString ip = QString::fromStdString(interfacePair.second);
        interfaces.push_back(ip);
    }
    std::sort(interfaces.begin(), interfaces.end());
    interfaces.push_front(QString(gt("all")));
    for (const auto &interface : interfaces) {
        ui->IpChooser->addItem(interface);
    }
    ui->IpChooser->setCurrentText(KiwixApp::instance()->getSettingsManager()->getKiwixServerIpAddress());
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
        m_port = ui->PortChooser->text().toInt();
        m_ipAddress = ui->IpChooser->currentText();

        m_active = KiwixApp::instance()->runServer(
            (m_ipAddress != gt("all") ? m_ipAddress : "0.0.0.0"),
            m_port
        );

        if (!m_active) {
            QMessageBox messageBox;
            messageBox.critical(0,gt("error-title"),gt("error-launch-server-message"));
        }
    } else {
        KiwixApp::instance()->stopServer();
        m_active = false;
    }

    if (m_active) {
        // Update UI to display how to acces the server
        m_ipAddress = m_ipAddress != gt("all") ? m_ipAddress : QString::fromStdString(kiwix::getBestPublicIp());
        ui->IpAddress->setText("http://" + m_ipAddress + ":" + QString::number(m_port));
        ui->IpAddress->setReadOnly(true);
        ui->KiwixServerButton->setText(gt("stop-kiwix-server"));
        ui->KiwixServerText->setText(gt("kiwix-server-running-message"));
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->KiwixServerButton->setText(gt("start-kiwix-server"));
        ui->KiwixServerText->setText(gt("kiwix-server-description"));
        ui->stackedWidget->setCurrentIndex(0);
    }
}
