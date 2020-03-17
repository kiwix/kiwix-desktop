#include "kprofile.h"

#include "kiwixapp.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QWebEngineSettings>

KProfile::KProfile(QObject *parent) :
    QWebEngineProfile(parent)
{
    connect(this, &QWebEngineProfile::downloadRequested, this, &KProfile::startDownload);
    installUrlSchemeHandler("zim", &m_schemeHandler);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
}

void KProfile::startDownload(QWebEngineDownloadItem* download)
{
    QString defaultFileName = download->url().fileName();
    QString fileName = QFileDialog::getSaveFileName(KiwixApp::instance()->getMainWindow(),
                                                       gt("save-file-as-window-title"), defaultFileName);
    if (fileName.isEmpty()) {
        return;
    }
    QString extension = "." + download->url().url().section('.', -1);
    if (!fileName.endsWith(extension)) {
        fileName.append(extension);
    }
    download->setPath(fileName);
    connect(download, &QWebEngineDownloadItem::finished, this, &KProfile::downloadFinished);
    download->accept();
}

void KProfile::downloadFinished()
{
    QMessageBox msgBox;
    msgBox.setText(gt("download-finished-message"));
    msgBox.exec();
}
