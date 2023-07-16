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
    void init(int zoomPercent, const QString &downloadDir, const QString &monitorDir, const bool moveToTrash);
public Q_SLOTS:
    void resetDownloadDir();
    void browseDownloadDir();
    void browseMonitorDir();
    void clearMonitorDir();
    void setZoom(int zoomPercent);
    void setMoveToTrash(bool moveToTrash);
    void onDownloadDirChanged(const QString &dir);
    void onMonitorDirChanged(const QString &dir);
    void onZoomChanged(qreal zoomFactor);
    void onMoveToTrashChanged(bool moveToTrash);
private:
    bool confirmDialogDownloadDir(const QString& dir);
    bool confirmDialog(QString messageText, QString messageTitle);
    bool confirmDialogMonitorDir(const QString& dir);
    Ui::Settings *ui;
};

#endif // SETTINGSVIEW_H
