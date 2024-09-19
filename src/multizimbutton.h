#ifndef MULTIZIMBUTTON_H
#define MULTIZIMBUTTON_H

#include <QToolButton>

class QListWidget;
class QButtonGroup;
class QCheckBox;

class MultiZimButton : public QToolButton {
    Q_OBJECT

public:
    MultiZimButton(QWidget *parent = nullptr);

public slots:
    void update_display();
    QString getZimId() const;

private:
    QListWidget* mp_buttonList;
    QButtonGroup* mp_radioButtonGroup;
    QCheckBox* mp_selectAllButton;
    QString m_zimId;
};

#endif // MULTIZIMBUTTON_H
