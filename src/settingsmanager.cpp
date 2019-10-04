#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent), m_settingsViewDisplayed(false)
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
    emit(portChanged(port));
}

void SettingsManager::setSettings()
{
    m_kiwixServerPort = 8181;
}