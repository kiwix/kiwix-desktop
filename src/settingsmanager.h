#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include "settingsmanagerview.h"

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int kiwixServerPort READ getKiwixServerPort NOTIFY portChanged)
    Q_PROPERTY(qreal zoomFactor READ getZoomFactor NOTIFY zoomChanged)
    Q_PROPERTY(QString downloadDir READ getDownloadDir NOTIFY downloadDirChanged)
    Q_PROPERTY(QString profileDir READ getProfileDir NOTIFY profileDirChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);
    virtual ~SettingsManager() {};

    SettingsManagerView* getView();
    bool isSettingsViewdisplayed() { return m_settingsViewDisplayed; };
    void setSettings(const QString &key, const QVariant &value);
    void deleteSettings(const QString &key);
    bool settingsExists(const QString &key);
    QVariant getSettings(const QString &key);
    qreal getZoomFactorByZimId(const QString &id);

public slots:
    void setKiwixServerPort(int port);
    int getKiwixServerPort() { return m_kiwixServerPort; };
    void setZoomFactor(qreal zoomFactor);
    qreal getZoomFactor() { return m_zoomFactor; };
    void validDownloadDir(QString dir);
    void setDownloadDir(QString downloadDir);
    QString getDownloadDir() { return m_downloadDir; }
    void resetDownloadDir();
    void browseDownloadDir();
    void validProfileDir(QString dir);
    void setProfileDir(QString profileDir);
    QString getProfileDir() { return m_profileDir; }
    void resetProfileDir();
    void browseProfileDir();
    void moveProfileFiles(QString newDir);

private:
    void initSettings();

signals:
    void portChanged(int port);
    void zoomChanged(qreal zoomFactor);
    void downloadDirChanged(QString downloadDir);
    void profileDirChanged(QString profileDir);
    void downloadDirChecked(bool valid);
    void profileDirChecked(bool valid);
    void profileFilesMoved(QString dir);

private:
    QSettings m_settings;
    bool m_settingsViewDisplayed;
    int m_kiwixServerPort;
    qreal m_zoomFactor;
    QString m_downloadDir;
    QString m_profileDir;
};

#endif // SETTINGSMANAGER_H
