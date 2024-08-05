#include "kiwixapp.h"
#include "texttospeechbar.h"
#include "ui_texttospeechbar.h"
#include <QDebug>

TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent), mp_speech(new QTextToSpeech(this)),
      mp_ui(new Ui::TextToSpeechBar)
{
    close();
    mp_ui->setupUi(this);
    mp_speech->setLocale(QLocale::system().language());
    mp_ui->stopButton->setText(gt("stop"));
    connect(mp_speech, &QTextToSpeech::stateChanged, this,
            &TextToSpeechBar::onStateChanged);
    connect(mp_ui->stopButton, &QPushButton::released, this,
            &TextToSpeechBar::stop);
    connect(mp_ui->closeButton, &QPushButton::released,
                this, &TextToSpeechBar::speechBarClose);
}

void TextToSpeechBar::speak(const QString &text)
{
    mp_speech->say(text);
}

void TextToSpeechBar::stop()
{
    mp_speech->stop();
}

void TextToSpeechBar::onStateChanged(QTextToSpeech::State state)
{
    mp_ui->stopButton->setEnabled(state != QTextToSpeech::Ready);
}

void TextToSpeechBar::speechBarClose()
{
    /* Prevent webview from scrolling to up to the top after losing focus. */
    auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;
    current->setFocus();

    mp_speech->stop();
    close();
}
