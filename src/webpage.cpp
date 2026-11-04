#include "webpage.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include "kiwixapp.h"
#include <QWebEngineProfile>

WebPage::WebPage(QObject *parent) :
    QWebEnginePage(KiwixApp::instance()->getProfile(), parent)
{
    action(QWebEnginePage::SavePage)->setVisible(false);
    action(QWebEnginePage::ViewSource)->setVisible(false);
    action(QWebEnginePage::Reload)->setVisible(false);

    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &WebPage::handleFeaturePermissionRequested);
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool /*isMainFrame*/)
{
    if (url.scheme() == "zim") {
        return true;
    }

    switch (type) {
    case QWebEnginePage::NavigationTypeLinkClicked:
    case QWebEnginePage::NavigationTypeFormSubmitted:
        QDesktopServices::openUrl(url);
        break;
    default:
        qDebug() << "Blocked navigation request to URL: " << url.toString();
        break;
    }
    return false;
}

void WebPage::handleFeaturePermissionRequested(const QUrl &origin, Feature feature)
{
    const bool isMediaRequest = feature == QWebEnginePage::MediaAudioCapture
        || feature == QWebEnginePage::MediaVideoCapture
        || feature == QWebEnginePage::MediaAudioVideoCapture;
    if (!isMediaRequest) {
        return;
    }

    const bool isCameraRequest = feature == QWebEnginePage::MediaVideoCapture;
    if (!isCameraRequest
        || origin.scheme() != QStringLiteral("zim")
        || origin.host().isEmpty()) {
        setFeaturePermission(origin, feature, QWebEnginePage::PermissionDeniedByUser);
        return;
    }

    const auto answer = QMessageBox::question(
        KiwixApp::instance()->getMainWindow(),
        gt("camera-permission-title"),
        gt("camera-permission-text").arg(origin.toString()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    const auto policy = answer == QMessageBox::Yes
        ? QWebEnginePage::PermissionGrantedByUser
        : QWebEnginePage::PermissionDeniedByUser;
    setFeaturePermission(origin, feature, policy);
}
