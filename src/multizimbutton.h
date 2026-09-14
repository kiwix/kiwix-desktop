#ifndef MULTIZIMBUTTON_H
#define MULTIZIMBUTTON_H

#include <QToolButton>

class QListWidget;
class QButtonGroup;
class QListWidgetItem;
class QCheckBox;
class QLabel;

class ZimItemWidget : public QWidget {
    Q_OBJECT

public:
    ZimItemWidget(QString text, QIcon icon, QWidget *parent = nullptr);

    QCheckBox* getCheckBox() const { return checkBx; }
    bool isChecked() const;

public slots:
    void setChecked(bool checked);
    void toggle();

private:
    QLabel* textLabel;
    QLabel* iconLabel;
    QCheckBox* checkBx;
};

class MultiZimButton : public QToolButton {
    Q_OBJECT

public:
    explicit MultiZimButton(QWidget *parent = nullptr);

public slots:
    void updateDisplay();
    void updateBooks();
    QStringList getZimIds() const;
    QStringList getCheckedZimIds() const;

private:
    QListWidget* mp_buttonList;

    ZimItemWidget* getZimWidget(int row) const;
    ZimItemWidget* getZimWidget(QListWidgetItem *item) const;
    void setItemZimWidget(QListWidgetItem* item, const QString& title, const QIcon& icon);
};

#endif // MULTIZIMBUTTON_H
