#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kiwixwebview.h"
#include "ktabwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    KTabWidget* getTabWidget();

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
