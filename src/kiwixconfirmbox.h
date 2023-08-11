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
    explicit KiwixConfirmBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent = nullptr);
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

#endif // KIWIXCONFIRMBOX_H
