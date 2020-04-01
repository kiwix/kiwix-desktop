#ifndef FINDINPAGEBAR_H
#define FINDINPAGEBAR_H

#include <QFrame>
#include <QLineEdit>

namespace Ui {
class FindInPageBar;
}

class FindInPageBar : public QFrame
{
    Q_OBJECT

public:
    explicit FindInPageBar(QWidget *parent = nullptr);
    ~FindInPageBar();

    QLineEdit* getFindLineEdit() { return mp_findLineEdit; };

public slots:
    void findNext();
    void findPrevious();
    void findClose();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::FindInPageBar *mp_ui;
    QLineEdit   *mp_findLineEdit;
};

#endif // FINDINPAGEBAR_H
