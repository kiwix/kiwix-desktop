#ifndef TABLEOFCONTENTBAR_H
#define TABLEOFCONTENTBAR_H

#include <QFrame>

namespace Ui {
class tableofcontentbar;
}

class TableOfContentBar : public QFrame
{
    Q_OBJECT

public:
    explicit TableOfContentBar(QWidget *parent = nullptr);
    ~TableOfContentBar();

private:
    Ui::tableofcontentbar *ui;
};

#endif // TABLEOFCONTENTBAR_H
