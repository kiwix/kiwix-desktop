#ifndef TOCSIDEBAR_H
#define TOCSIDEBAR_H

#include <QWidget>
#include <QLineEdit>

namespace Ui {
class TocSideBar;
}

class TocSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit TocSideBar(QWidget *parent = 0);
    ~TocSideBar();

    void postInit();
    QLineEdit* getFindLineEdit() { return mp_findLineEdit; };

public slots:
    void findNext();
    void findPrevious();
    void findClose();

private:
    Ui::TocSideBar *mp_ui;
    QLineEdit      *mp_findLineEdit;
};

#endif // TOCSIDEBAR_H
