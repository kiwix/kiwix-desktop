#ifndef KIWIXMESSAGEBOX_H
#define KIWIXMESSAGEBOX_H

#include <QDialog>

#include <stdexcept>

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

#endif // KIWIXMESSAGEBOX_H
