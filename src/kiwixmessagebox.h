#ifndef KIWIXCONFIRMBOX_H
#define KIWIXCONFIRMBOX_H

#include <QDialog>

namespace Ui {
class kiwixmessagebox;
}

class KiwixMessageBox : public QDialog
{
    Q_OBJECT

public:
    KiwixMessageBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent = nullptr);
    ~KiwixMessageBox();

signals:
    void yesClicked();
    void noClicked();
    void okClicked();

private:
    QString m_confirmTitle;
    QString m_confirmText;
    Ui::kiwixmessagebox *ui;
};


void showInfoBox(QString title, QString text, QWidget *parent = nullptr);

template<class YesAction>
void showConfirmBox(QString title, QString text, QWidget *parent,
                    YesAction yesAction)
{
    KiwixMessageBox *dialog = new KiwixMessageBox(title, text, false, parent);
    dialog->show();
    QObject::connect(dialog, &KiwixMessageBox::yesClicked, [=]() {
        yesAction();
        dialog->deleteLater();
    });
    QObject::connect(dialog, &KiwixMessageBox::noClicked, [=]() {
        dialog->deleteLater();
    });
}

#endif // KIWIXCONFIRMBOX_H
