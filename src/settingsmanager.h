#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include "settingsmanagerview.h"

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);
    virtual ~SettingsManager() {};

    SettingsManagerView* getView();
    bool isSettingsViewdisplayed() { return m_settingsViewDisplayed; };

private:
    bool m_settingsViewDisplayed;
};

#endif // SETTINGSMANAGER_H
