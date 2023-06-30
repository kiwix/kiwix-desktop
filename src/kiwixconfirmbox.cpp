#include "kiwixconfirmbox.h"
#include "ui_kiwixconfirmbox.h"
#include <QFile>
#include <QDebug>

KiwixConfirmBox::KiwixConfirmBox(QString confirmTitle, QString confirmText, QWidget *parent) :
    QDialog(parent), m_confirmTitle(confirmTitle), m_confirmText(confirmText),
    ui(new Ui::kiwixconfirmbox)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint, true);

    QFile styleFile(":/css/confirmBox.css");
    styleFile.open(QIODevice::ReadOnly);
    auto byteContent = styleFile.readAll();
    styleFile.close();
    QString style(byteContent);
    setStyleSheet(style);
    connect(ui->yesButton, &QPushButton::clicked, [=]() {
        emit yesClicked();
    });
    connect(ui->noButton, &QPushButton::clicked, [=]() {
        emit noClicked();
    });
    ui->confirmText->setText(confirmText);
    ui->confirmTitle->setText(confirmTitle);
}

KiwixConfirmBox::~KiwixConfirmBox()
{
    delete ui;
}
