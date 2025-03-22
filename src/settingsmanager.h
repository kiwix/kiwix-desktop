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
    typedef QList<QPair<QString, QString>> FilterList;
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
    bool getReopenTab() const { return m_reopenTab; }
    FilterList getLanguageList() { return deducePair(m_langList); }
    QStringList getCategoryList() { return m_categoryList; }
    FilterList getContentType() { return deducePair(m_contentTypeList); }
    bool getAutoCheckUpdates() const { return m_autoCheckUpdates; }

public slots:
    void setKiwixServerPort(int port);
    void setKiwixServerIpAddress(QString ipAddress);
    void setZoomFactor(qreal zoomFactor);
    void setDownloadDir(QString downloadDir);
    void setMonitorDir(QString monitorDir);
    void setMoveToTrash(bool moveToTrash);
    void setReopenTab(bool reopenTab);
    void setLanguage(FilterList langList);
    void setCategory(QStringList categoryList);
    void setContentType(FilterList contentTypeList);
    void setAutoCheckUpdates(bool enabled);

private:
    void initSettings();
    QList<QVariant> flattenPair(FilterList pairList);
    FilterList deducePair(QList<QVariant>);

signals:
    void portChanged(int port);
    void zoomChanged(qreal zoomFactor);
    void downloadDirChanged(QString downloadDir);
    void monitorDirChanged(QString monitorDir);
    void moveToTrashChanged(bool moveToTrash);
    void reopenTabChanged(bool reopenTab);
    void languageChanged(QList<QVariant> langList);
    void categoryChanged(QStringList categoryList);
    void contentTypeChanged(QList<QVariant> contentTypeList);

private:
    QSettings m_settings;
    SettingsView *m_view;
    int m_kiwixServerPort;
    QString m_kiwixServerIpAddress;
    qreal m_zoomFactor;
    QString m_downloadDir;
    QString m_monitorDir;
    bool m_moveToTrash;
    bool m_reopenTab;
    QList<QVariant> m_langList;
    QStringList m_categoryList;
    QList<QVariant> m_contentTypeList;
    bool m_autoCheckUpdates;
    static const QString SETTING_AUTO_CHECK_UPDATES;
};

QString getDataDirectory();
bool isPortableMode();

#endif // SETTINGSMANAGER_H
