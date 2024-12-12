#ifndef READINGLISTBAR_H
#define READINGLISTBAR_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class readinglistbar;
}

class ReadingListBar : public QFrame
{
    Q_OBJECT

public:
    explicit ReadingListBar(QWidget *parent = nullptr);
    ~ReadingListBar();

public slots:
    void setupList();
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem *item);
    void onItemPressed(QListWidgetItem* item, Qt::MouseButtons buttons);
    void onItemActivated(QListWidgetItem *item, Qt::MouseButtons buttons);
    void onExport();
    void onImport();
private:
    Ui::readinglistbar *ui;
    int clickKind;
    void openUrl(QListWidgetItem* item, bool newTab);
};

#endif // READINGLISTBAR_H
