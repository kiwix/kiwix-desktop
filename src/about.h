#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUT_H
