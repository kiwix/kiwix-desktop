#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class VersionChecker : public QObject
{
    Q_OBJECT // This macro is required for signals/slots to work

public:
    explicit VersionChecker(QObject* parent = nullptr);
    void checkForUpdates();
    static bool isNewerVersion(const QString& current, const QString& latest);

signals:
    void updateAvailable(const QString& latestVersion);
    void noUpdateAvailable();
    void checkFailed(const QString& error);

private slots:
    void handleNetworkReply(QNetworkReply* reply);

private:
    QNetworkAccessManager m_networkManager;
    const QString VERSION_CHECK_URL = "https://api.github.com/repos/kiwix/kiwix-desktop/releases/latest";
};

#endif // VERSIONCHECKER_H
