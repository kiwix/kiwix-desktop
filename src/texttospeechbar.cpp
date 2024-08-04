#include "texttospeechbar.h"
#include <QDebug>

TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent), mp_speech(new QTextToSpeech(this))
{
    mp_speech->setVolume(0.5);
    mp_speech->setLocale(QLocale::system().language());
}

void TextToSpeechBar::speak(const QString &text)
{
    mp_speech->say(text);
}
