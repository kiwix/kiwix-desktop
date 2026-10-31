#include "choiceitem.h"
#include "ui_choiceitem.h"
#include <QFile>
#include <QMouseEvent>
#include "kiwixapp.h"

ChoiceItem::ChoiceItem(QString key, QString value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChoiceItem),
    m_key(key),
    m_value(value)
{
    ui->setupUi(this);
    auto applyStyle = [this]() {
        this->setStyleSheet(themeAwareCss(":/css/choiceBox.css"));
    };
    applyStyle();
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::themeChanged,
            this, [applyStyle](SettingsManager::Theme) { applyStyle(); });
    ui->itemLabel->setText(key);
    ui->itemLabel->setToolTip(key);
    connect(ui->closeButton, &QPushButton::clicked, [=](){
       emit(closeButtonClicked(ui->itemLabel->text()));
    });
    ui->closeButton->setCursor(Qt::PointingHandCursor);
}

ChoiceItem::~ChoiceItem()
{
    delete ui;
}

void ChoiceItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}
