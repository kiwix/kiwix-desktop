#include "kprofile.h"

#include "kiwixapp.h"
#include <QFileDialog>
#include <QWebEngineSettings>
#include <QDesktopServices>
#include <QTemporaryFile>
#include "kiwixmessagebox.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define DownloadFinishedSignal WebEngineDownloadType::finished
#else
    #define DownloadFinishedSignal WebEngineDownloadType::isFinishedChanged
#endif

KProfile::KProfile(QObject *parent) :
    QWebEngineProfile(parent)
{
    connect(this, &QWebEngineProfile::downloadRequested, this, &KProfile::startDownload);
    installUrlSchemeHandler("zim", &m_schemeHandler);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0) // Earlier than Qt 5.13
    setRequestInterceptor(new ExternalReqInterceptor(this));
#else // Qt 5.13 and later
    setUrlRequestInterceptor(new ExternalReqInterceptor(this));
#endif
}

namespace {
    void setDownloadFilePath(KProfile::WebEngineDownloadType* download, QString filePath)
    {
    #if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        download->setPath(filePath);
    #else
        download->setDownloadFileName(filePath);
    #endif
    }
}

void KProfile::openFile(WebEngineDownloadType* download)
{
    QString defaultFileName = download->url().fileName();
    QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX." + QFileInfo(defaultFileName).suffix());
    tempFile.setAutoRemove(false);
    if (tempFile.open()) {
        QString tempFilePath = tempFile.fileName();
        tempFile.close();
        setDownloadFilePath(download, tempFilePath);
        connect(download, &DownloadFinishedSignal, [tempFilePath]() {
            if(!QDesktopServices::openUrl(QUrl::fromLocalFile(tempFilePath)))
                showInfoBox(gt("error-title"), gt("error-opening-file"), KiwixApp::instance()->getMainWindow());
        });
        download->accept();
    } else {
        qDebug()<<"tmp file err";
        download->cancel();
    }
}

void KProfile::saveFile(WebEngineDownloadType* download)
{
    QString defaultFileName = download->url().fileName();
    QString fileName = QFileDialog::getSaveFileName(KiwixApp::instance()->getMainWindow(), gt("save-file-as-window-title"),
                                               QDir::cleanPath(KiwixApp::instance()->getZimImportDir() + QDir::separator() + defaultFileName));
    if (fileName.isEmpty()) {
        download->cancel();
        return;
    }
    QString extension = "." + download->url().url().section('.', -1);
    if (!fileName.endsWith(extension)) {
        fileName.append(extension);
    }
    setDownloadFilePath(download, fileName);
    QFileInfo fileInfo(fileName);
    KiwixApp::instance()->setZimImportDir(fileInfo.absolutePath());
    connect(download, &DownloadFinishedSignal, [=]() {
        showInfoBox(gt("download-finished"), gt("download-finished-message"), KiwixApp::instance()->getMainWindow());
    });
    download->accept();
}

void KProfile::startDownload(WebEngineDownloadType* download)
{
    auto res = showKiwixMessageBox(gt("save-or-open"), gt("save-or-open-text"),
                            KiwixApp::instance()->getMainWindow(), gt("save-file"), gt("open-file"));
    if (res == KiwixMessageBox::YesClicked) {
        saveFile(download);
        return;
    }
    if (res == KiwixMessageBox::NoClicked) {
        openFile(download);
        return;
    }
    download->cancel();
}

void ExternalReqInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    const QString reqUrl = info.requestUrl().toString();
    if (!reqUrl.startsWith("zim://"))
    {
        qDebug() << "Blocked external request to URL: " << reqUrl;
        info.block(true);
    }
}
