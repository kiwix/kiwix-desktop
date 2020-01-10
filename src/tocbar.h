#ifndef TOCBAR_H
#define TOCBAR_H

#include <QDockWidget>
#include <QLineEdit>

namespace Ui {
class TocBar;
}

class TocBar : public QDockWidget
{
    Q_OBJECT

public:
    explicit TocBar(QWidget *parent = nullptr);
    ~TocBar();

    QLineEdit* getFindLineEdit() { return mp_findLineEdit; };

public slots:
    void findNext();
    void findPrevious();
    void findClose();
    void findCloseByIndex(int index);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::TocBar  *mp_ui;
    QLineEdit   *mp_findLineEdit;
};

#endif // TOCBAR_H
