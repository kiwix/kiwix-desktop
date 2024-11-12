#include "texttospeechbar.h"

TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent)
{
    m_speech.setLocale(QLocale::system().language());
}

void TextToSpeechBar::speak(const QString &text)
{
    m_speech.say(text);
}
