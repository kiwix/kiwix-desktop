#ifndef SETTINGSMANAGERVIEW_H
#define SETTINGSMANAGERVIEW_H

#include <QWebEngineView>
#include <QWebChannel>

class SettingsManagerView : public QWebEngineView
{
public:
    SettingsManagerView(QWidget *parent = nullptr);
    void registerObject(const QString &id, QObject *object);
    void setHtml();

private:
    QWebChannel m_webChannel;
};

#endif // SETTINGSMANAGERVIEW_H
