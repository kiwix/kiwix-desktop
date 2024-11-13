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
    Q_OBJECT
public:
    explicit ComboBoxLineEdit(QWidget *parent = 0);

public slots:
    void preventSelection();

protected:
    void mouseReleaseEvent(QMouseEvent *);
};

class TextToSpeechBar : public QFrame
{
    Q_OBJECT
public:
    explicit TextToSpeechBar(QWidget *parent = nullptr);

    void speak(const QString& text);
    void stop();

    void setupLanguageComboBox();
    void setupVoiceComboBox();
    void resetVoiceComboBox();

public slots:
    void onStateChanged(QTextToSpeech::State state);
    void speechClose();
    void speechShow();
    void languageSelected(int index);
    void voiceSelected(int index);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTextToSpeech m_speech;
    Ui::TextToSpeechBar *ui;
    QVector<QVoice> m_voices;
};

#endif // TEXTTOSPEECHMANAGER_H
