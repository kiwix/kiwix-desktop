#include "webpage.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include "kiwixapp.h"
#include <QWebEngineProfile>

WebPage::WebPage(QObject *parent) :
    QWebEnginePage(parent)
{
    action(QWebEnginePage::SavePage)->setVisible(false);
    action(QWebEnginePage::ViewSource)->setVisible(false);
    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, this, &WebPage::startDownload);
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (url.scheme() != "zim") {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        for (auto i = 1; i < tabWidget->count() - 1; i++) {
            if (tabWidget->widget(i)->url().isEmpty()) {
                tabWidget->closeTab(i);
            }
        }
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}

void WebPage::startDownload(QWebEngineDownloadItem* download)
{
    QString fileName = QFileDialog::getSaveFileName(KiwixApp::instance()->getMainWindow(),
                                                       tr("Save File as"));
    if (fileName.isEmpty()) {
        return;
    }
    QString extension = "." + download->url().url().section('.', -1);
    if (!fileName.endsWith(extension)) {
        fileName.append(extension);
    }
    download->setPath(fileName);
    connect(download, &QWebEngineDownloadItem::finished, this, &WebPage::downloadFinished);
    download->accept();
}

void WebPage::downloadFinished()
{
    QMessageBox msgBox;
    msgBox.setText(tr("The document has been downloaded."));
    msgBox.exec();
}