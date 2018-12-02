#ifndef READINGLISTBAR_H
#define READINGLISTBAR_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class readinglistbar;
}

class ReadingListBar : public QWidget
{
    Q_OBJECT

public:
    explicit ReadingListBar(QWidget *parent = nullptr);
    ~ReadingListBar();

public slots:
    void setupList();
    void on_itemActivated(QListWidgetItem *item);
private:
    Ui::readinglistbar *ui;
};

#endif // READINGLISTBAR_H
