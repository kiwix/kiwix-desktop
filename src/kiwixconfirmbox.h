#ifndef KIWIXCONFIRMBOX_H
#define KIWIXCONFIRMBOX_H

#include <QDialog>

#include <stdexcept>

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

class KiwixAppError : public std::runtime_error
{
public:
    KiwixAppError(const QString& summary, const QString& details)
        : std::runtime_error(summary.toStdString())
        , m_details(details)
    {}

    QString summary() const { return QString::fromStdString(what()); }
    QString details() const { return m_details; }

private:
    QString m_details;
};

inline void showErrorBox(const KiwixAppError& err, QWidget *parent = nullptr)
{
    showInfoBox(err.summary(), err.details(), parent);
}

#endif // KIWIXCONFIRMBOX_H
