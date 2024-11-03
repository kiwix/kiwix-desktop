#ifndef MULTIZIMBUTTON_H
#define MULTIZIMBUTTON_H

#include <QToolButton>
#include <QCheckBox>

class QListWidget;
class QButtonGroup;
class QListWidgetItem;
class QRadioButton;
class QLabel;

class SelectAllButton : public QCheckBox {
    Q_OBJECT

public:
    SelectAllButton(const QString &text, QWidget *parent = nullptr) : QCheckBox(text, parent) {}

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

class ZimItemWidget : public QWidget {
    Q_OBJECT

public:
    ZimItemWidget(QString text, QIcon icon, QWidget *parent = nullptr);

    QRadioButton* getRadioButton() const { return radioBt; }

private:
    QLabel* textLabel;
    QLabel* iconLabel;
    QRadioButton* radioBt;
};

class MultiZimButton : public QToolButton {
    Q_OBJECT

public:
    explicit MultiZimButton(QWidget *parent = nullptr);

public slots:
    void updateDisplay();
    QStringList getZimIds() const;

private:
    QListWidget* mp_buttonList;
    QButtonGroup* mp_radioButtonGroup;
    SelectAllButton* mp_selectAllButton;

    ZimItemWidget* getZimWidget(int row) const;
    void setItemZimWidget(QListWidgetItem* item, const QString& title, const QIcon& icon);
};

#endif // MULTIZIMBUTTON_H
