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

    if (isNewerVersion(version, latestVersion)) {
        emit updateAvailable(latestVersion);
    } else {
        emit noUpdateAvailable();
    }

    reply->deleteLater();
}

void VersionChecker::parseReleaseDirectory(const QString& html)
{
    QList<ReleaseInfo> releases;
    QRegularExpression linkRegex(QString("href=\"(%1)\".*?(\\d{2}-\\w{3}-\\d{4} \\d{2}:\\d{2})")
                                .arg(getPlatformSpecificPattern()));
    
    auto matches = linkRegex.globalMatch(html);
    while (matches.hasNext()) {
        auto match = matches.next();
        ReleaseInfo info;
        info.filename = match.captured(1);
        info.downloadUrl = DOWNLOAD_BASE_URL + info.filename;
        info.releaseDate = QDateTime::fromString(match.captured(2), "dd-MMM-yyyy hh:mm");
        
        // Extract version from filename
        QRegularExpression versionRegex("\\d+\\.\\d+\\.\\d+");
        auto vMatch = versionRegex.match(info.filename);
        if (vMatch.hasMatch()) {
            info.version = vMatch.captured(0);
            releases.append(info);
        }
    }

    if (releases.isEmpty()) {
        emit checkFailed("No compatible releases found");
        return;
    }

    auto latestRelease = findLatestRelease(releases);
    if (isNewerVersion(version, latestRelease.version)) {
        emit updateAvailable(latestRelease.version);
    } else {
        emit noUpdateAvailable();
    }
}

QString VersionChecker::getPlatformSpecificPattern()
{
#if defined(Q_OS_WIN)
    return "kiwix-desktop_.*_windows-x86_64\\.zip";
#elif defined(Q_OS_LINUX)
    return "kiwix-desktop_.*_linux-x86_64\\.appimage";
#else
    return "kiwix-desktop_.*\\.(appimage|zip)";
#endif
}

QString VersionChecker::getDownloadUrl(const QString& version) const
{
    QString filename = QString("kiwix-desktop");
#if defined(Q_OS_WIN)
    filename += QString("_windows-x86_64_%1.zip").arg(version);
#elif defined(Q_OS_LINUX)
    filename += QString("_x86_64_%1.appimage").arg(version);
#endif
    return DOWNLOAD_BASE_URL + filename;
}

void VersionChecker::downloadAndInstallUpdate(const QString& url, const QString& /*version*/)  // Mark version as unused
{
    logDebug("Starting download from: " + url);
    m_downloadPath = getDownloadDirectory() + QDir::separator() + 
                    QFileInfo(url).fileName();

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, 
                       QNetworkRequest::NoLessSafeRedirectPolicy);
    
    m_downloadReply = m_networkManager.get(request);
    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &VersionChecker::handleDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &VersionChecker::handleDownloadFinished);
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

void VersionChecker::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void VersionChecker::handleDownloadFinished()
{
    if (m_downloadReply->error() != QNetworkReply::NoError) {
        emit installationFailed(m_downloadReply->errorString());
        m_downloadReply->deleteLater();
        return;
    }

    QFile file(m_downloadPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit installationFailed("Could not write to file: " + file.errorString());
        m_downloadReply->deleteLater();
        return;
    }

    file.write(m_downloadReply->readAll());
    file.close();
    m_downloadReply->deleteLater();

    // Verify package integrity
    if (!verifyDownloadedPackage(m_downloadPath)) {
        emit installationFailed("Package verification failed");
        return;
    }

    emit installationStarted();
    
    // Try to install the downloaded update
    if (!installUpdate(m_downloadPath)) {
        emit installationFailed("Failed to install update");
        return;
    }

    emit installationFinished(true);
}

bool VersionChecker::installUpdate(const QString& filePath)
{
#if defined(Q_OS_WIN)
    // For Windows, extract zip and run installer
    // TODO: Implement Windows update installation
    return false;
#elif defined(Q_OS_LINUX)
    // For Linux, make AppImage executable and replace current one
    QFile file(filePath);
    if (!file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return false;
    }
    
    // Get path of current executable
    QString currentExePath = QCoreApplication::applicationFilePath();
    QString backupPath = currentExePath + ".backup";
    
    // Backup current executable
    if (!QFile::copy(currentExePath, backupPath)) {
        return false;
    }
    
    // Replace current executable with new one
    if (!QFile::rename(filePath, currentExePath)) {
        // Restore backup if replacement fails
        QFile::copy(backupPath, currentExePath);
        return false;
    }
    
    // Remove backup
    QFile::remove(backupPath);
    
    return true;
#else
    return false;
#endif
}

QString VersionChecker::getDownloadDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

VersionChecker::ReleaseInfo VersionChecker::findLatestRelease(const QList<ReleaseInfo>& releases)  // Add VersionChecker:: scope
{
    ReleaseInfo latest;
    latest.version = "0.0.0";
    latest.releaseDate = QDateTime::fromSecsSinceEpoch(0);

    for (const auto& release : releases) {
        if (isNewerVersion(latest.version, release.version) ||
            (latest.version == release.version && latest.releaseDate < release.releaseDate)) {
            latest = release;
        }
    }

    return latest;
}

bool VersionChecker::verifyDownloadedPackage(const QString& filePath) const
{
    logDebug("Verifying downloaded package: " + filePath);
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.size() > 0;
}

void VersionChecker::logDebug(const QString& message) const
{
    qDebug() << "[VersionChecker]" << message;
}
