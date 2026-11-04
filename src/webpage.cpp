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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    connect(this, &QWebEnginePage::permissionRequested,
            this, &WebPage::handlePermissionRequested);
#else
    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &WebPage::handleFeaturePermissionRequested);
#endif
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
void WebPage::handlePermissionRequested(const QWebEnginePermission &permission)
{
    const auto permissionType = permission.permissionType();
    const bool isMediaRequest =
        permissionType == QWebEnginePermission::PermissionType::MediaAudioCapture
        || permissionType == QWebEnginePermission::PermissionType::MediaVideoCapture
        || permissionType == QWebEnginePermission::PermissionType::MediaAudioVideoCapture;
    if (!isMediaRequest) {
        return;
    }

    const QUrl origin = permission.origin();
    const bool isCameraRequest =
        permissionType == QWebEnginePermission::PermissionType::MediaVideoCapture;
    if (!isCameraRequest
        || origin.scheme() != QStringLiteral("zim")
        || origin.host().isEmpty()) {
        permission.deny();
        return;
    }

    const auto answer = QMessageBox::question(
        KiwixApp::instance()->getMainWindow(),
        gt("camera-permission-title"),
        gt("camera-permission-text").arg(origin.toString()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer == QMessageBox::Yes) {
        permission.grant();
    } else {
        permission.deny();
    }
}
#else
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
#endif
