#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include "webview.h"
#include "tabbar.h"
#include "topwidget.h"
#include "about.h"
#include "contentmanagerside.h"
#include "localkiwixserver.h"

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
    QWidget getMainView();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void toggleFullScreen();
    void tabChanged(TabBar::TabType);
    void readingListToggled(bool state);
    void hideTabAndTop();
    void showTabAndTop();
    void updateTabButtons();
    void togglePrivateMode();

private:
    Ui::MainWindow *mp_ui;
    About     *mp_about;
    LocalKiwixServer *mp_localKiwixServer;
};

#endif // MAINWINDOW_H
