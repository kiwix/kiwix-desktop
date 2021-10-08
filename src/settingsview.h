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
    void init(int port, int factor, const QString &dir);
public Q_SLOTS:
    void resetDownloadDir();
    void browseDownloadDir();
    void zoomFactorChanged(int factor);
    void setDownloadDir(const QString &dir);
    void setZoom(qreal factor);
    void setKiwixServerPort(int port);
    void serverPortChanged(int port);
private:
    bool confirmDialogDownloadDir(const QString& dir);

    Ui::Settings *ui;
};

#endif // SETTINGSVIEW_H
