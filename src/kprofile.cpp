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

void KProfile::setFileOpenChoice(QString fileName, bool choice)
{
    m_fileOpenChoice = std::make_pair(fileName,choice);
}

KProfile::DownloadInfo KProfile::getFileOpenChoice() const
{
    return m_fileOpenChoice;
}

bool KProfile::isNonReadableFile(QString mimeType)
{
    if (mimeType.toStdString()=="application/pdf"){ 
        return true;
    }
    return false;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QString KProfile::getFileName(QWebEngineDownloadItem* download)
#else
    QString KProfile::getFileName(QWebEngineDownloadRequest* download)
#endif
{
    int ret;
    bool nrFile = isNonReadableFile(download->mimeType());
    QString defaultFileName = download->url().fileName();
    QString fileName("");
    
    if(nrFile){
        QMessageBox msgBox;

        msgBox.setText(gt("download-or-open"));
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Save | QMessageBox::Close);
        msgBox.button(QMessageBox::Save )->setText(gt("download"));
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();

        if(ret == QMessageBox::Open){
            QTemporaryFile tempFile;

            if (tempFile.open()) {
                fileName = tempFile.fileName();
                tempFile.close();
            }
            setFileOpenChoice(fileName,true);
        }
    }

    if(!nrFile || ret == QMessageBox::Save){
        fileName = QFileDialog::getSaveFileName(KiwixApp::instance()->getMainWindow(),
                                                            gt("save-file-as-window-title"), defaultFileName);
        if (!fileName.isEmpty()) {
            QString extension = "." + download->url().url().section('.', -1);
            if (!fileName.endsWith(extension)) {
                fileName.append(extension);
            }
            setFileOpenChoice(fileName,false);
        }
        
    }
    return fileName;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void KProfile::startDownload(QWebEngineDownloadItem* download)
#else
void KProfile::startDownload(QWebEngineDownloadRequest* download)
#endif
{   
    QString fileName=getFileName(download);

    if (fileName.isEmpty()) {
            return;
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
    KProfile::DownloadInfo dInfo = getFileOpenChoice();

    if (dInfo.second){
        QDesktopServices::openUrl(QUrl::fromLocalFile(dInfo.first));
        return;
    }
    
    QMessageBox msgBox;
    msgBox.setText(gt("download-finished-message"));
    msgBox.exec();
}