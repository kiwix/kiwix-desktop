#include "settingsview.h"
#include "ui_settings.h"
#include "kiwixapp.h"
#include <kiwix/tools.h>
#include <QMessageBox>
#include <QFileDialog>
SettingsView::SettingsView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    QFile file(QString::fromUtf8(":/css/_settingsManager.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString(file.readAll());
    ui->widget->setStyleSheet(styleSheet);
    connect(ui->serverPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::serverPortChanged);
    connect(ui->zoomPercentSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::setZoom);
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsView::browseDownloadDir);
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingsView::resetDownloadDir);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::downloadDirChanged, this, &SettingsView::setDownloadDir);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::zoomChanged, this, &SettingsView::onZoomChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::portChanged, this, &SettingsView::setKiwixServerPort);
    ui->settingsLabel->setText(gt("settings"));
    ui->serverPortLabel->setText(gt("port-for-local-kiwix-server-setting"));
    ui->zoomPercentLabel->setText(gt("zoom-level-setting"));
    ui->downloadDirLabel->setText(gt("download-directory-setting"));
    ui->resetButton->setText(gt("reset"));
    ui->browseButton->setText(gt("browse"));
}
void SettingsView::init(int port, int zoomFactor, const QString &dir)
{
    ui->serverPortSpinBox->setValue(port);
    ui->zoomPercentSpinBox->setValue(zoomFactor);
    ui->downloadDirPath->setText(dir);
}
bool SettingsView::confirmDialogDownloadDir(const QString& dir)
{
    auto text = gt("download-dir-dialog-msg");
    text = text.replace("{{DIRECTORY}}", dir);
    QMessageBox msgBox(
        QMessageBox::Question, //Icon
        gt("download-dir-dialog-title"), //Title
        text, //Text
        QMessageBox::Ok | QMessageBox::Cancel //Buttons
    );
    msgBox.setDefaultButton(QMessageBox::Ok);

    int ret = msgBox.exec();
    return (ret == QMessageBox::Ok);
}

void SettingsView::resetDownloadDir()
{
    auto dir = QString::fromStdString(kiwix::getDataDirectory());
    const auto &downloadDir = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    if (dir == downloadDir) {
        return;
    }
    if (confirmDialogDownloadDir(dir)) {
        KiwixApp::instance()->getSettingsManager()->setDownloadDir(dir);
    }
}

void SettingsView::browseDownloadDir()
{
    const auto &downloadDir = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    QString dir = QFileDialog::getExistingDirectory(KiwixApp::instance()->getMainWindow(),
                                                    gt("browse-directory"),
                                                    downloadDir,
                                                    QFileDialog::ShowDirsOnly);
    if (dir == downloadDir || dir.isEmpty()) {
        return;
    }

    if (confirmDialogDownloadDir(dir)) {
        KiwixApp::instance()->getSettingsManager()->setDownloadDir(dir);
    }
}

void SettingsView::setZoom(int zoomPercent)
{
    qreal zoomFactor = (qreal) zoomPercent/100;
    KiwixApp::instance()->getSettingsManager()->setZoomFactor(zoomFactor);
}

void SettingsView::serverPortChanged(int port)
{
    KiwixApp::instance()->getSettingsManager()->setKiwixServerPort(port);
}

void SettingsView::setDownloadDir(const QString &dir)
{
    ui->downloadDirPath->setText(dir);
}

void SettingsView::onZoomChanged(qreal zoomFactor)
{
    qreal zoomPercent = zoomFactor * 100;
    ui->zoomPercentSpinBox->setValue(zoomPercent);
}

void SettingsView::setKiwixServerPort(int port)
{
    ui->serverPortSpinBox->setValue(port);
}
