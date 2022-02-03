#include "settingsmanager.h"
#include "kiwixapp.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <kiwix/tools.h>
SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
    m_settings("Kiwix", "Kiwix-desktop"),
    m_view(nullptr)
{
    initSettings();
}

SettingsView* SettingsManager::getView()
{
    if (m_view == nullptr) {
        auto view = new SettingsView();
        view->init(m_zoomFactor * 100, m_downloadDir, m_monitorDir);
        connect(view, &QObject::destroyed, this, [=]() { m_view = nullptr; });
        m_view = view;
    }
    return m_view;
}

void SettingsManager::setSettings(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

void SettingsManager::deleteSettings(const QString &key)
{
    m_settings.remove(key);
}

bool SettingsManager::settingsExists(const QString &key)
{
    return m_settings.contains(key);
}

QVariant SettingsManager::getSettings(const QString &key)
{
    return m_settings.value(key);
}

qreal SettingsManager::getZoomFactorByZimId(const QString &id)
{
    auto zoomFactor = m_zoomFactor;
    QString key = id + "/zoomFactor";
    if (settingsExists(key)) {
        zoomFactor = getSettings(key).toDouble();
    }
    return zoomFactor;
}

void SettingsManager::setKiwixServerPort(int port)
{
    m_kiwixServerPort = port;
    m_settings.setValue("localKiwixServer/port", port);
    emit(portChanged(port));
}

void SettingsManager::setKiwixServerIpAddress(QString ipAddress)
{
    m_kiwixServerIpAddress = ipAddress;
    m_settings.setValue("localKiwixServer/ipAddress", ipAddress);
}

void SettingsManager::setZoomFactor(qreal zoomFactor)
{
    m_zoomFactor = zoomFactor;
    m_settings.setValue("view/zoomFactor", zoomFactor);
    emit(zoomChanged(zoomFactor));
}

void SettingsManager::setDownloadDir(QString downloadDir)
{
    m_downloadDir = downloadDir;
    m_settings.setValue("download/dir", downloadDir);
    emit(downloadDirChanged(downloadDir));
}

void SettingsManager::setMonitorDir(QString monitorDir)
{
    m_monitorDir = monitorDir;
    m_settings.setValue("monitor/dir", monitorDir);
    emit(monitorDirChanged(monitorDir));
}

void SettingsManager::initSettings()
{
    m_kiwixServerPort = m_settings.value("localKiwixServer/port", 8080).toInt();
    m_zoomFactor = m_settings.value("view/zoomFactor", 1).toDouble();
    m_downloadDir = m_settings.value("download/dir", QString::fromStdString(kiwix::getDataDirectory())).toString();
    m_kiwixServerIpAddress = m_settings.value("localKiwixServer/ipAddress", QString("0.0.0.0")).toString();
    m_monitorDir = m_settings.value("monitor/dir", QString("")).toString();
}
