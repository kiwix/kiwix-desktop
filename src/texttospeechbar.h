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
    explicit TextToSpeechBar(QWidget *parent = nullptr);

    void speak(const QString& text);
    void stop();

public slots:
    void onStateChanged(QTextToSpeech::State state);
    void speechClose();
    void speechShow();

private:
    QTextToSpeech m_speech;
    Ui::TextToSpeechBar *ui;
};

#endif // TEXTTOSPEECHMANAGER_H
