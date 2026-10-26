#include "kprofile.h"

#include "kiwixapp.h"
#include "kiwixmessagebox.h"

#include <QFileDialog>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QDesktopServices>
#include <QTemporaryFile>

namespace
{

QWebEngineScript getScript(QString filename,
    QWebEngineScript::InjectionPoint point = QWebEngineScript::DocumentReady)
{
    QWebEngineScript script;
    script.setInjectionPoint(point);
    script.setWorldId(QWebEngineScript::UserWorld);

    script.setSourceCode(getFileContent(filename));
    return script;
}

QString getExtensionFromFilter(const QString &filter) {
  const QRegularExpression re("\\(\\*\\.(\\w+)\\)");
  const QRegularExpressionMatch match = re.match(filter);

  if (match.hasMatch()) {
    return "." + match.captured(1);
  }
  return QString();
}

}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define DownloadFinishedSignal WebEngineDownloadType::finished
    #define DownloadInProgress QWebEngineDownloadItem::DownloadInProgress
#else
    #define DownloadFinishedSignal WebEngineDownloadType::isFinishedChanged
    #define DownloadInProgress QWebEngineDownloadRequest::DownloadInProgress
#endif

QString askForSaveFilePath(const QString &suggestedName, const QString &filter) {
  const auto app = KiwixApp::instance();
  const QString suggestedPath = app->getPrevSaveDir() + "/" + suggestedName;
  QString selectedFilter;
  QString fileName = QFileDialog::getSaveFileName(
      app->getMainWindow(), gt("save-file-as-window-title"), suggestedPath,
      filter,&selectedFilter);

  if (fileName.isEmpty())
    return QString();

  const QString extension = getExtensionFromFilter(selectedFilter);
  if (!fileName.endsWith(extension)) {
    fileName.append(extension);
  }
  app->savePrevSaveDir(QFileInfo(fileName).absolutePath());
  return fileName;
}

void downloadFinished()
{
    showInfoBox(gt("download-finished"),
                gt("download-finished-message"),
                KiwixApp::instance()->getMainWindow()
    );
}

void articleDownloadFinished(const QString &title,const QString &filePath)
{
  QString finishedMessage = gt("article-download-finished-message");
  finishedMessage = finishedMessage.replace("{{TITLE}}", title);
  finishedMessage = finishedMessage.replace("{{PATH}}", filePath);

  showInfoBox(gt("article-download-finished"), finishedMessage,
              KiwixApp::instance()->getMainWindow());
}

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

    scripts()->insert(getScript(":/js/headerAnchor.js"));
    scripts()->insert(getScript(":/qtwebchannel/qwebchannel.js",
                      QWebEngineScript::DocumentCreation));
}

namespace
{

void setDownloadFilePath(WebEngineDownloadType* download, QString filePath)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    // WebEngineDownloadType is QWebEngineDownloadItem
    // and no QWebEngineDownloadItem::setDownloadFileName() yet
    download->setPath(filePath);
#else
    // Same API for QWebEngineDownloadItem and QWebEngineDownloadRequest
    download->setDownloadFileName(filePath);
#endif
}

QString getDownloadFilePath(WebEngineDownloadType* download)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    // WebEngineDownloadType is QWebEngineDownloadItem
    // and no QWebEngineDownloadItem::downloadFileName() yet
    return QUrl(download->path()).fileName();
#else
    // Same API for QWebEngineDownloadItem and QWebEngineDownloadRequest
    return download->downloadFileName();
#endif
}

} // unnamed namespace

void KProfile::openFile(WebEngineDownloadType* download)
{
    const QString defaultFileName = getDownloadFilePath(download);
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
    const QString defaultFileName = getDownloadFilePath(download);
    const QString extension = defaultFileName.section(".", -1);
    const QString filter = extension.isEmpty() ? "" : "(*." + extension + ")";
    const QString fileName = askForSaveFilePath(defaultFileName, filter);
    if (fileName.isEmpty()) {
        download->cancel();
        return;
    }

    setDownloadFilePath(download, fileName);
    connect(download, &DownloadFinishedSignal, &downloadFinished);
    download->accept();
}

void KProfile::startDownload(WebEngineDownloadType* download)
{
  if (download->state() == DownloadInProgress) {
    connect(download, &DownloadFinishedSignal, [download]() {
      const QString fullPath = QDir(download->downloadDirectory())
                             .filePath(download->downloadFileName());
      articleDownloadFinished(download->page()->title(), fullPath);
    });
    return;
  }

    const auto res = showKiwixMessageBox(
                            gt("save-or-open"),
                            gt("save-or-open-text"),
                            KiwixApp::instance()->getMainWindow(),
                            gt("save-file"),
                            gt("open-file")
    );

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
