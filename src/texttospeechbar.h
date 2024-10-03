#ifndef TEXTTOSPEECHMANAGER_H
#define TEXTTOSPEECHMANAGER_H

#include <QTextToSpeech>
#include <QFrame>
#include <QLineEdit>

namespace Ui {
class TextToSpeechBar;
}

class ComboBoxLineEdit : public QLineEdit
{
public:
    ComboBoxLineEdit(QWidget *parent = 0) : QLineEdit(parent){}
protected:
    void mouseReleaseEvent(QMouseEvent *);
};

class TextToSpeechBar : public QFrame
{
    Q_OBJECT
public:
    TextToSpeechBar(QWidget *parent = nullptr);

    void speak(const QString& text);
    void stop();
    void setLocale(QLocale locale);

public slots:
    void speechBarClose();
    void languageSelected(int index);
    void voiceSelected(int index);
    void onStateChanged(QTextToSpeech::State state);

private:
    QTextToSpeech m_speech;
    Ui::TextToSpeechBar *mp_ui;
    QVector<QVoice> m_voices;
    QString m_text;
};

#endif // TEXTTOSPEECHMANAGER_H
