#ifndef TEXTTOSPEECHMANAGER_H
#define TEXTTOSPEECHMANAGER_H

#include <QTextToSpeech>
#include <QFrame>
class TextToSpeechBar : public QFrame
{
    Q_OBJECT
public:
    TextToSpeechBar(QWidget *parent = nullptr);

    void speak(const QString& text);

private:
    QTextToSpeech *mp_speech;
};

#endif // TEXTTOSPEECHMANAGER_H
