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
signals:
    void serverPortChanged(int port);
    void zoomFactorChanged(int factor);
    void downloadDirChanged(const QString &dir);
private:
    bool confirmDialogDownloadDir(const QString& dir);

    Ui::Settings *ui;
};

#endif // SETTINGSVIEW_H
