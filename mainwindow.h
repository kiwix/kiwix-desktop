#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kiwixwebview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void displayReader(std::shared_ptr<kiwix::Reader> reader);

private:
    Ui::MainWindow *ui;
    std::map<std::shared_ptr<kiwix::Reader>, KiwixWebView*> webviews_map;
};

#endif // MAINWINDOW_H
