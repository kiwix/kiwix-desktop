#ifndef KIWIXCONFIRMBOX_H
#define KIWIXCONFIRMBOX_H

#include <QDialog>

namespace Ui {
class kiwixconfirmbox;
}

class KiwixConfirmBox : public QDialog
{
    Q_OBJECT

public:
    KiwixConfirmBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent = nullptr);
    ~KiwixConfirmBox();

signals:
    void yesClicked();
    void noClicked();
    void okClicked();

private:
    QString m_confirmTitle;
    QString m_confirmText;
    Ui::kiwixconfirmbox *ui;
};


void showInfoBox(QString title, QString text, QWidget *parent = nullptr);

template<class YesAction>
void showConfirmBox(QString title, QString text, QWidget *parent,
                    YesAction yesAction)
{
    KiwixConfirmBox *dialog = new KiwixConfirmBox(title, text, false, parent);
    dialog->show();
    QObject::connect(dialog, &KiwixConfirmBox::yesClicked, [=]() {
        yesAction();
        dialog->deleteLater();
    });
    QObject::connect(dialog, &KiwixConfirmBox::noClicked, [=]() {
        dialog->deleteLater();
    });
}

// template<class YesAction, class NoAction>
// void showConfirmBox(QString title, QString text, QWidget *parent,
//                     YesAction yesAction, NoAction noAction)
// {
//     KiwixConfirmBox *dialog = new KiwixConfirmBox(title, text, false, parent);
//     dialog->show();
//     QObject::connect(dialog, &KiwixConfirmBox::yesClicked, [=]() {
//         yesAction();
//         dialog->deleteLater();
//     });
//     QObject::connect(dialog, &KiwixConfirmBox::noClicked, [=]() {
//         noAction();
//         dialog->deleteLater();
//     });
// }


#endif // KIWIXCONFIRMBOX_H
