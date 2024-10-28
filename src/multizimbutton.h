#ifndef MULTIZIMBUTTON_H
#define MULTIZIMBUTTON_H

#include <QToolButton>

class QListWidget;
class QButtonGroup;

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
};

#endif // MULTIZIMBUTTON_H
