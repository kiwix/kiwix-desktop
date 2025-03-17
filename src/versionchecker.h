#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class VersionChecker : public QObject
{
Q_OBJECT // This macro is required for signals/slots to work

    public : struct ReleaseInfo
    {
        QString version;
        QString downloadUrl;
        QString filename;
        QDateTime releaseDate;
    };

    explicit VersionChecker(QObject *parent = nullptr);
    void checkForUpdates();
    void downloadAndInstallUpdate(const QString &url, const QString &version);
    static bool isNewerVersion(const QString &current, const QString &latest);
    QString getDownloadUrl(const QString &version) const;

signals:
    void updateAvailable(const QString &latestVersion);
    void noUpdateAvailable();
    void checkFailed(const QString &error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void installationStarted();
    void installationFinished(bool success);
    void installationFailed(const QString &error);

private slots:
    void handleNetworkReply(QNetworkReply *reply);
    void parseReleaseDirectory(const QString &html);
    void handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void handleDownloadFinished();

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_downloadReply;
    QString m_downloadPath;
    const QString VERSION_CHECK_URL = "https://api.github.com/repos/kiwix/kiwix-desktop/releases/latest";
    const QString DOWNLOAD_BASE_URL = "https://download.kiwix.org/release/kiwix-desktop/";
    ReleaseInfo findLatestRelease(const QList<ReleaseInfo> &releases);
    bool installUpdate(const QString &filePath);
    QString getPlatformSpecificPattern();
    QString getDownloadDirectory();
};

#endif // VERSIONCHECKER_H
