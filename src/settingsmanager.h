#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include "settingsview.h"

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int kiwixServerPort READ getKiwixServerPort NOTIFY portChanged)
    Q_PROPERTY(qreal zoomFactor MEMBER m_zoomFactor WRITE setZoomFactor NOTIFY zoomChanged)
    Q_PROPERTY(QString downloadDir MEMBER m_downloadDir WRITE setDownloadDir NOTIFY downloadDirChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);
    virtual ~SettingsManager() {};

    SettingsView* getView();
    void setSettings(const QString &key, const QVariant &value);
    void deleteSettings(const QString &key);
    bool settingsExists(const QString &key);
    QVariant getSettings(const QString &key);
    qreal getZoomFactorByZimId(const QString &id);
    int getKiwixServerPort() const { return m_kiwixServerPort; }
    QString getKiwixServerIpAddress() const { return m_kiwixServerIpAddress; }
    qreal getZoomFactor() const { return m_zoomFactor; }
    QString getDownloadDir() const { return m_downloadDir; }
    QString getMonitorDir() const { return m_monitorDir; }
    bool getMoveToTrash() const { return m_moveToTrash; }
    QStringList getLanguageList() const { return m_langList; }
    QStringList getCategoryList() const { return m_categoryList; }

public slots:
    void setKiwixServerPort(int port);
    void setKiwixServerIpAddress(QString ipAddress);
    void setZoomFactor(qreal zoomFactor);
    void setDownloadDir(QString downloadDir);
    void setMonitorDir(QString monitorDir);
    void setMoveToTrash(bool moveToTrash);
    void setLanguage(QStringList langList);
    void setCategory(QStringList categoryList);
private:
    void initSettings();

signals:
    void portChanged(int port);
    void zoomChanged(qreal zoomFactor);
    void downloadDirChanged(QString downloadDir);
    void monitorDirChanged(QString monitorDir);
    void moveToTrashChanged(bool moveToTrash);
    void languageChanged(QStringList langList);
    void categoryChanged(QStringList categoryList);

private:
    QSettings m_settings;
    SettingsView *m_view;
    int m_kiwixServerPort;
    QString m_kiwixServerIpAddress;
    qreal m_zoomFactor;
    QString m_downloadDir;
    QString m_monitorDir;
    bool m_moveToTrash;
    QStringList m_langList;
    QStringList m_categoryList;
};

#endif // SETTINGSMANAGER_H
