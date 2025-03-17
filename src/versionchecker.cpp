#include "versionchecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include "kiwixapp.h" // Add this to get access to version

VersionChecker::VersionChecker(QObject* parent)
    : QObject(parent)
{
    connect(&m_networkManager, &QNetworkAccessManager::finished,
            this, &VersionChecker::handleNetworkReply);
}

void VersionChecker::checkForUpdates()
{
    QNetworkRequest request(VERSION_CHECK_URL);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Kiwix-Desktop-Version-Checker");
    m_networkManager.get(request);
}

void VersionChecker::handleNetworkReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    const auto data = reply->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isObject()) {
        emit checkFailed("Invalid response format");
        reply->deleteLater();
        return;
    }

    const auto obj = doc.object();
    const auto tagName = obj["tag_name"].toString();
    
    // Remove 'v' prefix if present
    QString latestVersion = tagName;
    if (latestVersion.startsWith('v', Qt::CaseInsensitive)) {
        latestVersion.remove(0, 1);
    }

    if (isNewerVersion(version, latestVersion)) { // Use version from kiwixapp.h
        emit updateAvailable(latestVersion);
    } else {
        emit noUpdateAvailable();
    }

    reply->deleteLater();
}

bool VersionChecker::isNewerVersion(const QString& current, const QString& latest)
{
    const QRegularExpression versionRegex("(\\d+)\\.(\\d+)\\.(\\d+)");
    auto currentMatch = versionRegex.match(current);
    auto latestMatch = versionRegex.match(latest);
    
    if (!currentMatch.hasMatch() || !latestMatch.hasMatch()) {
        return false;
    }

    for (int i = 1; i <= 3; ++i) {
        int currentNum = currentMatch.captured(i).toInt();
        int latestNum = latestMatch.captured(i).toInt();
        
        if (latestNum > currentNum) {
            return true;
        }
        if (latestNum < currentNum) {
            return false;
        }
    }
    
    return false;
}
