#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "contentmanagerside.h"
#include "localkiwixserver.h"
#include "webview.h"
#include "tabbar.h"
#include "topwidget.h"
#include "about.h"

#include <QMainWindow>
#include <QDockWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    TabBar*   getTabBar();
    TopWidget* getTopWidget();
    QStackedWidget* getSideDockWidget();
    ContentManagerSide* getSideContentManager();

protected slots:
    void toggleFullScreen();
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *mp_ui;
    About     *mp_about;
    LocalKiwixServer *mp_localKiwixServer;
};

#endif // MAINWINDOW_H
