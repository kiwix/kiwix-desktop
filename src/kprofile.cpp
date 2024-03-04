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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void KProfile::startDownload(QWebEngineDownloadItem* download)
#else
void KProfile::startDownload(QWebEngineDownloadRequest* download)
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    download->setPath(fileName);
#else
    download->setDownloadFileName(fileName);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(download, &QWebEngineDownloadItem::finished, this, &KProfile::downloadFinished);
#else
    connect(download, &QWebEngineDownloadRequest::isFinished, this, &KProfile::downloadFinished);
#endif
    download->accept();
}

void KProfile::downloadFinished()
{
    QMessageBox msgBox;
    msgBox.setText(gt("download-finished-message"));
    msgBox.exec();
}
