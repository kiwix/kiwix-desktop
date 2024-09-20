#ifndef MULTIZIMBUTTON_H
#define MULTIZIMBUTTON_H

#include <QToolButton>

class QListWidget;
class QListWidgetItem;
class QButtonGroup;
class QCheckBox;
class QRadioButton;
class QLabel;

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
    MultiZimButton(QWidget *parent = nullptr);

public slots:
    void update_display();
    QString getZimId() const;

private:
    QListWidget* mp_buttonList;
    QButtonGroup* mp_radioButtonGroup;
    QCheckBox* mp_selectAllButton;
    QString m_zimId;

    ZimItemWidget* getZimWidget(QListWidgetItem* item) const;
};

#endif // MULTIZIMBUTTON_H
