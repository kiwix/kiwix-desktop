#ifndef FINDINPAGEBAR_H
#define FINDINPAGEBAR_H

#include <QWidget>

namespace Ui {
class FindInPageBar;
}

class FindInPageBar : public QWidget
{
    Q_OBJECT

public:
    explicit FindInPageBar(QWidget *parent = nullptr);
    ~FindInPageBar();

private:
    Ui::FindInPageBar *ui;
};

#endif // FINDINPAGEBAR_H
