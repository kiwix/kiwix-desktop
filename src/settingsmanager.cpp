#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
    m_settings("Kiwix", "Kiwix-desktop"),
    m_settingsViewDisplayed(false)
{
    setSettings();
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

void SettingsManager::setKiwixServerPort(int port)
{
    m_kiwixServerPort = port;
    m_settings.setValue("localKiwixServer/port", port);
    emit(portChanged(port));
}

void SettingsManager::setSettings()
{
    m_kiwixServerPort = m_settings.value("localKiwixServer/port", 8181).toInt();
}