#include "settingsview.h"
#include "ui_settings.h"
#include "kiwixapp.h"

#include <QMessageBox>
#include <QFileDialog>
SettingsView::SettingsView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    connect(ui->serverPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::serverPortChanged);
    connect(ui->zoomLevelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsView::zoomFactorChanged);
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingsView::resetDownloadDir);
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsView::browseDownloadDir);
    ui->settingsLabel->setText(gt("settings"));
    ui->serverPortLabel->setText(gt("port-for-local-kiwix-server-setting"));
    ui->zoomLevelLabel->setText(gt("zoom-level-setting"));
    ui->downloadDirLabel->setText(gt("download-directory-setting"));
    ui->resetButton->setText(gt("reset"));
    ui->browseButton->setText(gt("browse"));
}
void SettingsView::init(int port, int factor, const QString &dir)
{
    ui->serverPortSpinBox->setValue(port);
    ui->zoomLevelSpinBox->setValue(factor);
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
    auto dir = QString::fromStdString(getDataDirectory());
    const auto &downloadDir = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    if (dir == downloadDir) {
        return;
    }
    if (confirmDialogDownloadDir(dir)) {
      ui->downloadDirPath->setText(dir);
      emit(downloadDirChanged(dir));
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
      ui->downloadDirPath->setText(dir);
      emit(downloadDirChanged(dir));
    }
}
