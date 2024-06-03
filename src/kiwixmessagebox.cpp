#include "kiwixmessagebox.h"
#include "ui_kiwixmessagebox.h"
#include <QFile>
#include "kiwixapp.h"

KiwixMessageBox::KiwixMessageBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent) :
    QDialog(parent), m_confirmTitle(confirmTitle), m_confirmText(confirmText),
    ui(new Ui::kiwixmessagebox)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setStyleSheet(KiwixApp::instance()->parseStyleFromFile(":/css/confirmBox.css"));
    connect(ui->yesButton, &QPushButton::clicked, [=]() {
        emit yesClicked();
    });
    connect(ui->noButton, &QPushButton::clicked, [=]() {
        emit noClicked();
    });
    connect(ui->okButton, &QPushButton::clicked, [=]() {
        emit okClicked();
    });
    ui->confirmText->setText(confirmText);
    ui->confirmTitle->setText(confirmTitle);
    ui->yesButton->setText(gt("yes"));
    ui->noButton->setText(gt("no"));
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
