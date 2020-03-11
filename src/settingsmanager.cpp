#include "settingsmanager.h"
#include "kiwix/tools/pathTools.h"
#include "kiwixapp.h"
#include <QDir>
#include <QFileDialog>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
    m_settings("Kiwix", "Kiwix-desktop"),
    m_settingsViewDisplayed(false)
{
    initSettings();
}

SettingsManagerView* SettingsManager::getView()
{
    auto view = new SettingsManagerView();
    view->registerObject("settingsManager", this);
    view->setHtml();
    connect(view, &QObject::destroyed, this, [=]() { m_settingsViewDisplayed = false; });
    m_settingsViewDisplayed = true;
    return view;
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

void SettingsManager::setZoomFactor(qreal zoomFactor)
{
    m_zoomFactor = zoomFactor;
    m_settings.setValue("view/zoomFactor", zoomFactor);
}

void SettingsManager::validDownloadDir(QString dir)
{
    emit(downloadDirChecked(fileExists(dir.toStdString())));
}

void SettingsManager::setDownloadDir(QString downloadDir)
{
    m_downloadDir = downloadDir;
    m_settings.setValue("download/dir", downloadDir);
}

void SettingsManager::resetDownloadDir()
{
    emit(downloadDirChanged(QString::fromStdString(getDataDirectory())));
}

void SettingsManager::browseDownloadDir()
{
    QString dir = QFileDialog::getExistingDirectory(KiwixApp::instance()->getMainWindow(),
                                                    tr("Browse Directory"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly);
    emit(downloadDirChanged(dir));
}

void SettingsManager::validProfileDir(QString dir)
{
    emit(profileDirChecked(fileExists(dir.toStdString())));
}

void SettingsManager::setProfileDir(QString profileDir)
{
    m_profileDir = profileDir;
    m_settings.setValue("profile/dir", profileDir);
}

void SettingsManager::resetProfileDir()
{
    emit(profileDirChanged(QString::fromStdString(getDataDirectory())));
}

void SettingsManager::browseProfileDir()
{
    QString dir = QFileDialog::getExistingDirectory(KiwixApp::instance()->getMainWindow(),
                                                    tr("Browse Directory"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly);
    emit(profileDirChanged(dir));
}

void SettingsManager::moveProfileFiles(QString newDir)
{
    QFile libraryFile(m_profileDir + "/library.xml");
    qInfo() << libraryFile.rename(newDir + "/library.xml");
    
    QFile bookmarksFile(m_profileDir + "/library.bookmarks.xml");
    qInfo() << bookmarksFile.rename(newDir + "/library.bookmarks.xml");

    QFile kiwixSessionFile(m_profileDir + "/kiwix.session");
    qInfo() << kiwixSessionFile.rename(newDir + "/kiwix.session");
    emit(profileFilesMoved(newDir));
}

void SettingsManager::initSettings()
{
    m_kiwixServerPort = m_settings.value("localKiwixServer/port", 8181).toInt();
    m_zoomFactor = m_settings.value("view/zoomFactor", 1).toDouble();
    m_downloadDir = m_settings.value("download/dir", QString::fromStdString(getDataDirectory())).toString();
    m_profileDir = m_settings.value("profile/dir", QString::fromStdString(getDataDirectory())).toString();
}