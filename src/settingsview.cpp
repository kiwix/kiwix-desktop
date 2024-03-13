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
    ui->widget->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/_settingsManager.css"));
    connect(ui->zoomPercentSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::setZoom);
    connect(ui->moveToTrashToggle, &QCheckBox::clicked, this, &SettingsView::setMoveToTrash);
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsView::browseDownloadDir);
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingsView::resetDownloadDir);
    connect(ui->importBrowse, &QPushButton::clicked, this, &SettingsView::browseImportDir);
    connect(ui->importReset, &QPushButton::clicked, this, &SettingsView::resetImportDir);
    connect(ui->monitorBrowse, &QPushButton::clicked, this, &SettingsView::browseMonitorDir);
    connect(ui->monitorClear, &QPushButton::clicked, this, &SettingsView::clearMonitorDir);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::downloadDirChanged, this, &SettingsView::onDownloadDirChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::importDirChanged, this, &SettingsView::onImportDirChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::monitorDirChanged, this, &SettingsView::onMonitorDirChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::zoomChanged, this, &SettingsView::onZoomChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::moveToTrashChanged, this, &SettingsView::onMoveToTrashChanged);
    ui->settingsLabel->setText(gt("settings"));
    ui->zoomPercentLabel->setText(gt("zoom-level-setting"));
    ui->downloadDirLabel->setText(gt("download-directory-setting"));
    ui->importDirLabel->setText(gt("import-directory-setting"));
    ui->monitorDirLabel->setText(gt("monitor-directory-setting"));
    ui->resetButton->setText(gt("reset"));
    ui->browseButton->setText(gt("browse"));
    ui->importReset->setText(gt("reset"));
    ui->importBrowse->setText(gt("browse"));
    ui->monitorClear->setText(gt("clear"));
    ui->monitorBrowse->setText(gt("browse"));
    ui->monitorHelp->setText("<b>?</b>");
    ui->monitorHelp->setToolTip(gt("monitor-directory-tooltip"));
    ui->moveToTrashLabel->setText(gt("move-files-to-trash"));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->moveToTrashLabel->hide();
    ui->moveToTrashToggle->hide();
#endif

}

void SettingsView::init(int zoomPercent, const QString &downloadDir, const QString &importDir, const QString &monitorDir, const bool moveToTrash)
{
    ui->zoomPercentSpinBox->setValue(zoomPercent);
    ui->downloadDirPath->setText(downloadDir);
    ui->importDirPath->setText(importDir);
    if (monitorDir == QString()) {
        ui->monitorClear->hide();
    }
    ui->monitorDirPath->setText(monitorDir);
    ui->moveToTrashToggle->setChecked(moveToTrash);
}
bool SettingsView::confirmDialog( QString messageText, QString messageTitle)
{
    QMessageBox msgBox(
        QMessageBox::Question, //Icon
        messageTitle, //Title
        messageText, //Text
        QMessageBox::Ok | QMessageBox::Cancel //Buttons
    );
    msgBox.setDefaultButton(QMessageBox::Ok);

    int ret = msgBox.exec();
    return (ret == QMessageBox::Ok);
}

bool SettingsView::confirmDialogDownloadDir(const QString &dir) {
    auto messageText = gt("download-dir-dialog-msg");
    messageText = messageText.replace("{{DIRECTORY}}", dir);
    return confirmDialog(messageText, gt("download-dir-dialog-title"));
}

bool SettingsView::confirmDialogImportDir(const QString &dir) {
    auto messageText = gt("import-dir-dialog-msg");
    messageText = messageText.replace("{{DIRECTORY}}", dir);
    return confirmDialog(messageText, gt("import-dir-dialog-title"));
}

bool SettingsView::confirmDialogMonitorDir(const QString &dir) {
    auto messageText = gt("monitor-dir-dialog-msg");
    messageText = messageText.replace("{{DIRECTORY}}", dir);
    auto messageTitle = gt("monitor-dir-dialog-title");
    return confirmDialog(messageText, messageTitle);
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

void SettingsView::resetImportDir()
{
    auto dir = QString::fromStdString(kiwix::getDataDirectory());
    const auto &importDir = KiwixApp::instance()->getSettingsManager()->getImportDir();
    if (dir == importDir) {
        return;
    }
    if (confirmDialogImportDir(dir)) {
        KiwixApp::instance()->getSettingsManager()->setImportDir(dir);
    }
}

void SettingsView::browseImportDir()
{
    const auto &importDir = KiwixApp::instance()->getSettingsManager()->getImportDir();
    QString dir = QFileDialog::getExistingDirectory(KiwixApp::instance()->getMainWindow(),
                                                    gt("browse-directory"),
                                                    importDir,
                                                    QFileDialog::ShowDirsOnly);
    if (dir == importDir || dir.isEmpty()) {
        return;
    }

    if (confirmDialogImportDir(dir)) {
        KiwixApp::instance()->getSettingsManager()->setImportDir(dir);
    }
}

void SettingsView::browseMonitorDir()
{
    const auto &monitorDir = KiwixApp::instance()->getSettingsManager()->getMonitorDir();
    QString previousDir;
    if (monitorDir == "") {
        previousDir = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    } else {
        previousDir = monitorDir;
    }
    QString dir = QFileDialog::getExistingDirectory(KiwixApp::instance()->getMainWindow(),
                                                    gt("browse-directory"),
                                                    previousDir,
                                                    QFileDialog::ShowDirsOnly);
    if (dir == monitorDir || dir.isEmpty()) {
        return;
    }
    if (confirmDialogMonitorDir(dir)) {
        KiwixApp::instance()->setMonitorDir(dir);
    }
}

void SettingsView::clearMonitorDir()
{
    if (confirmDialog(gt("monitor-clear-dir-dialog-msg"), gt("monitor-clear-dir-dialog-title"))) {
        KiwixApp::instance()->setMonitorDir("");
    }
}

void SettingsView::setZoom(int zoomPercent)
{
    qreal zoomFactor = (qreal) zoomPercent/100;
    KiwixApp::instance()->getSettingsManager()->setZoomFactor(zoomFactor);
}

void SettingsView::setMoveToTrash(bool moveToTrash)
{
    KiwixApp::instance()->getSettingsManager()->setMoveToTrash(moveToTrash);
}

void SettingsView::onDownloadDirChanged(const QString &dir)
{
    ui->downloadDirPath->setText(dir);
}

void SettingsView::onImportDirChanged(const QString &dir)
{
    ui->importDirPath->setText(dir);
}

void SettingsView::onMonitorDirChanged(const QString &dir)
{
    if (dir == "") {
        ui->monitorClear->hide();
    } else {
        ui->monitorClear->show();
    }
    ui->monitorDirPath->setText(dir);
}

void SettingsView::onZoomChanged(qreal zoomFactor)
{
    qreal zoomPercent = zoomFactor * 100;
    ui->zoomPercentSpinBox->setValue(zoomPercent);
}

void SettingsView::onMoveToTrashChanged(bool moveToTrash)
{
    ui->moveToTrashToggle->setChecked(moveToTrash);
}
