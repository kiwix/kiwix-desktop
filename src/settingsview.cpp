#include "settingsview.h"
#include "ui_settings.h"
#include "kiwixapp.h"
#include <kiwix/tools.h>
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolTip>

namespace 
{
    QString formatSettingsDir(const QString& input) {
        const int maxLength = 40;
        if (input.length() > maxLength) {
            QString suffix = input.right(maxLength);
            int directoryIndex = suffix.indexOf('/');
            return "..." + suffix.mid(directoryIndex != -1 ? directoryIndex : 0);
        }
        return input;
    }
}

SettingsView::SettingsView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    SettingsManager *settingsMgr = KiwixApp::instance()->getSettingsManager();
    ui->setupUi(this);
    ui->widget->setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/_settingsManager.css"));
    connect(ui->zoomPercentSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::setZoom);
    connect(ui->moveToTrashToggle, &QCheckBox::clicked, this, &SettingsView::setMoveToTrash);
    connect(ui->reopenTabToggle, &QCheckBox::clicked, this, &SettingsView::setReopenTab);
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsView::browseDownloadDir);
    connect(ui->downloadDirPathCopy, &QPushButton::clicked, [this, settingsMgr]() {
        copySettingsPathToClipboard(settingsMgr->getDownloadDir(), ui->downloadDirPathCopy);
    });
    connect(ui->monitorDirPathCopy, &QPushButton::clicked, [this, settingsMgr]() {
        copySettingsPathToClipboard(settingsMgr->getMonitorDir(), ui->monitorDirPathCopy);
    });
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingsView::resetDownloadDir);
    connect(ui->monitorBrowse, &QPushButton::clicked, this, &SettingsView::browseMonitorDir);
    connect(ui->monitorClear, &QPushButton::clicked, this, &SettingsView::clearMonitorDir);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::downloadDirChanged, this, &SettingsView::onDownloadDirChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::monitorDirChanged, this, &SettingsView::onMonitorDirChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::zoomChanged, this, &SettingsView::onZoomChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::moveToTrashChanged, this, &SettingsView::onMoveToTrashChanged);
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::reopenTabChanged, this, &SettingsView::onReopenTabChanged);
    ui->settingsLabel->setText(gt("settings"));
    ui->zoomPercentLabel->setText(gt("zoom-level-setting"));
    ui->downloadDirLabel->setText(gt("download-directory-setting"));
    ui->monitorDirLabel->setText(gt("monitor-directory-setting"));
    ui->resetButton->setText(gt("reset"));
    ui->browseButton->setText(gt("browse"));
    ui->monitorClear->setText(gt("clear"));
    ui->monitorBrowse->setText(gt("browse"));
    QIcon copyIcon(":/icons/copy.svg");
    ui->downloadDirPathCopy->setIcon(copyIcon);
    ui->downloadDirPathCopy->setIconSize(QSize(24, 24));
    ui->monitorDirPathCopy->setIcon(copyIcon);
    ui->monitorDirPathCopy->setIconSize(QSize(24, 24));
    ui->monitorHelp->setText("<b>?</b>");
    ui->monitorHelp->setToolTip(gt("monitor-directory-tooltip"));
    ui->moveToTrashLabel->setText(gt("move-files-to-trash"));
    ui->reopenTabLabel->setText(gt("open-previous-tabs-at-startup"));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->line_5->hide();
    ui->moveToTrashLabel->hide();
    ui->moveToTrashToggle->hide();
#endif

}

void SettingsView::init(int zoomPercent, const QString &downloadDir,
                        const QString &monitorDir, const bool moveToTrash,
                        bool reopentab)
{
    ui->zoomPercentSpinBox->setValue(zoomPercent);
    SettingsView::onDownloadDirChanged(downloadDir);
    SettingsView::onMonitorDirChanged(monitorDir);
    ui->moveToTrashToggle->setChecked(moveToTrash);
    ui->reopenTabToggle->setChecked(reopentab);
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

void SettingsView::setReopenTab(bool reopen)
{
    KiwixApp::instance()->getSettingsManager()->setReopenTab(reopen);
}

void SettingsView::onDownloadDirChanged(const QString &dir)
{
    ui->downloadDirPath->setText(formatSettingsDir(dir));
    ui->downloadDirPath->setToolTip(dir);
}

void SettingsView::copySettingsPathToClipboard(QString pathToCopy, QPushButton* button)
{
    QApplication::clipboard()->setText(pathToCopy);
    QPoint globalPos = button->mapToGlobal(QPoint(0, -button->height()));
    QToolTip::showText(globalPos, gt("path-was-copied"), button);
}

void SettingsView::onMonitorDirChanged(const QString &dir)
{
    ui->monitorClear->setVisible(!dir.isEmpty());
    ui->monitorDirPathCopy->setVisible(!dir.isEmpty());
    ui->monitorDirPath->setText(formatSettingsDir(dir));
    ui->monitorDirPath->setToolTip(dir);
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

void SettingsView::onReopenTabChanged(bool reopen)
{
    ui->reopenTabToggle->setChecked(reopen);
}
