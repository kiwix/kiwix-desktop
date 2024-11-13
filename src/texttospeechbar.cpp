#include "texttospeechbar.h"
#include "kiwixapp.h"
#include "ui_texttospeechbar.h"

TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent), ui(new Ui::TextToSpeechBar)
{
    ui->setupUi(this);

    m_speech.setLocale(QLocale::system().language());
    connect(&m_speech, &QTextToSpeech::stateChanged, this,
            &TextToSpeechBar::onStateChanged);

    ui->stopButton->setText(gt("stop"));
    ui->stopButton->setDisabled(true);
    connect(KiwixApp::instance()->getAction(KiwixApp::ReadStopAction), &QAction::triggered,
            this, &TextToSpeechBar::stop);
    connect(ui->stopButton, &QPushButton::pressed, this,
            &TextToSpeechBar::stop);
    connect(ui->closeButton, &QPushButton::pressed,
                this, &TextToSpeechBar::speechClose);
}

void TextToSpeechBar::speak(const QString &text)
{
    m_speech.say(text);
}

void TextToSpeechBar::stop()
{
    m_speech.stop();
}

void TextToSpeechBar::speechClose()
{
    /* Prevent webview from scrolling to up to the top after losing focus. */
    const auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;

    current->setFocus();
    m_speech.stop();
    close();
}

void TextToSpeechBar::speechShow()
{
    show();
    setFocus();
}

void TextToSpeechBar::onStateChanged(QTextToSpeech::State state)
{
    ui->stopButton->setEnabled(state != QTextToSpeech::Ready);
}
