#include "kprofile.h"

#include "kiwixapp.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QWebEngineSettings>
#include <QDesktopServices>



KProfile::KProfile(QObject *parent) :
    QWebEngineProfile(parent)
{
    connect(this, &QWebEngineProfile::downloadRequested, this, &KProfile::startDownload);
    installUrlSchemeHandler("zim", &m_schemeHandler);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
}

SettingsManager* getSettingsManager()
{
    return KiwixApp::instance()->getSettingsManager();
}

void KProfile::setDownloadInfo(const QString &fileName, const QString &mimeType)
{
    m_downloadInfo = std::make_pair(fileName, mimeType);
}

KProfile::DownloadInfo KProfile::getDownloadInfo() const
{
    return m_downloadInfo;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void KProfile::startDownload(QWebEngineDownloadItem* download)
#else
void KProfile::startDownload(QWebEngineDownloadRequest* download)
#endif
{   
    QString fileName;
    QString defaultFileName = download->url().fileName();

    QString dmimeType = download->mimeType();
    
    if (dmimeType.toStdString()=="application/pdf"){ 
        
        auto downloadPath = getSettingsManager()->getDownloadDir();
        fileName = downloadPath + "/" + defaultFileName;
        setDownloadInfo(fileName,dmimeType);
        
        QFile file(fileName);

        if (file.exists()){
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            return;
        }
    }

    else{
        fileName = QFileDialog::getSaveFileName(KiwixApp::instance()->getMainWindow(),
                                                        gt("save-file-as-window-title"), defaultFileName);
        if (fileName.isEmpty()) {
            return;
        }
        QString extension = "." + download->url().url().section('.', -1);
        if (!fileName.endsWith(extension)) {
            fileName.append(extension);
        }
        setDownloadInfo(fileName,dmimeType);
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
    KProfile::DownloadInfo dInfo = getDownloadInfo();
    if (dInfo.second.toStdString()=="application/pdf"){
        QDesktopServices::openUrl(QUrl::fromLocalFile(dInfo.first));
    }
    QMessageBox msgBox;
    msgBox.setText(gt("download-finished-message"));
    msgBox.exec();
}