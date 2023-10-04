#ifndef KIWIXLISTWIDGET_H
#define KIWIXLISTWIDGET_H

#include <QListWidget>

class KiwixListWidget : public QListWidget {
    Q_OBJECT

public:
    KiwixListWidget(QWidget *parent = nullptr);
    void moveUp();
    void moveDown();
    void selectCurrent();
    void selectCurrent(QListWidgetItem *item);
    void setVisibleItems(int visibleItems) { m_visibleItems = visibleItems; }
    int getVisibleItems() { return m_visibleItems; }

protected:
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void currRowChanged(int oldRow, int newRow);

private slots:
    void handleCurrRowChange(int oldRow, int newRow);

private:
    int currRow;
    int m_visibleItems;
    int m_mouseIndex;
};

#endif // KIWIXLISTWIDGET_H
