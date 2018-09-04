#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include "webview.h"
#include "tabwidget.h"
#include "tocsidebar.h"
#include "about.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    TabWidget*   getTabWidget();
    QDockWidget* getSideDockWidget();

protected slots:
    void toggleFullScreen();

private:
    Ui::MainWindow *mp_ui;
    About     *mp_about;

};

#endif // MAINWINDOW_H
