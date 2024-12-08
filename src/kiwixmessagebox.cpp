#include "kiwixmessagebox.h"
#include "ui_kiwixmessagebox.h"
#include <QFile>
#include "kiwixapp.h"

KiwixMessageBox::KiwixMessageBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent,
                                 QString leftAction, QString rightAction) :
    QDialog(parent), m_confirmTitle(confirmTitle), m_confirmText(confirmText),
    ui(new Ui::kiwixmessagebox)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/messageBox.css"));
    connect(ui->yesButton, &QPushButton::clicked, [=]() {
        emit yesClicked();
        m_result = YesClicked;
        accept();
    });
    connect(ui->noButton, &QPushButton::clicked, [=]() {
        emit noClicked();
        m_result = NoClicked;
        reject();
    });
    connect(ui->okButton, &QPushButton::clicked, [=]() {
        emit okClicked();
        m_result = OkClicked;
    });
    connect(ui->closeButton, &QPushButton::clicked, [=]() {
        this->close();
        m_result = CloseClicked;
    });
    ui->confirmText->setText(confirmText);
    ui->confirmTitle->setText(confirmTitle);
    ui->yesButton->setText(leftAction);
    ui->noButton->setText(rightAction);
    ui->okButton->setText(gt("ok"));
    ui->okButton->hide();
    if (okDialog) {
        ui->yesButton->hide();
        ui->noButton->hide();
        ui->okButton->show();
    }
}

KiwixMessageBox::~KiwixMessageBox()
{
    delete ui;
}

void showInfoBox(QString title, QString text, QWidget *parent)
{
    KiwixMessageBox *dialog = new KiwixMessageBox(title, text, true, parent);
    dialog->show();
    QObject::connect(dialog, &KiwixMessageBox::okClicked, [=]() {
        dialog->deleteLater();
    });
}

KiwixMessageBox::Result showKiwixMessageBox(QString title, QString text, QWidget *parent, QString leftTitle, QString rightTitle)
{
    KiwixMessageBox *dialog = new KiwixMessageBox(title, text, false, parent, leftTitle, rightTitle);
    QObject::connect(dialog, &KiwixMessageBox::finished, [=]() {
        dialog->deleteLater();
    });
    return dialog->execDialog();
}
