#include "kiwixconfirmbox.h"
#include "ui_kiwixconfirmbox.h"
#include <QFile>
#include "kiwixapp.h"

KiwixConfirmBox::KiwixConfirmBox(QString confirmTitle, QString confirmText, bool okDialog, QWidget *parent) :
    QDialog(parent), m_confirmTitle(confirmTitle), m_confirmText(confirmText),
    ui(new Ui::kiwixconfirmbox)
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

KiwixConfirmBox::~KiwixConfirmBox()
{
    delete ui;
}
