#ifndef TEXTTOSPEECHMANAGER_H
#define TEXTTOSPEECHMANAGER_H

#include <QTextToSpeech>
#include <QFrame>

namespace Ui {
class TextToSpeechBar;
}

class TextToSpeechBar : public QFrame
{
    Q_OBJECT
public:
    TextToSpeechBar(QWidget *parent = nullptr);

    void speak(const QString& text);
    void stop();

public slots:
    void speechBarClose();
    void languageSelected(int index);
    void onStateChanged(QTextToSpeech::State state);

private:
    QTextToSpeech *mp_speech;
    Ui::TextToSpeechBar *mp_ui;
};

#endif // TEXTTOSPEECHMANAGER_H
