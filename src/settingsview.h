#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
namespace Ui {
class Settings;
}
class SettingsView : public QWidget
{
    Q_OBJECT
public:
    SettingsView(QWidget *parent = nullptr);
    ~SettingsView(){};
    void init(int port, int zoomPercent, const QString &dir);
public Q_SLOTS:
    void resetDownloadDir();
    void browseDownloadDir();
    void setZoom(int zoomPercent);
    void onDownloadDirChanged(const QString &dir);
    void onZoomChanged(qreal zoomFactor);
    void onServerPortChanged(int port);
    void setKiwixServerPort(int port);
private:
    bool confirmDialogDownloadDir(const QString& dir);

    Ui::Settings *ui;
};

#endif // SETTINGSVIEW_H
