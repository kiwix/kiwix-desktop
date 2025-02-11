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
    void setLocale(const QLocale& locale);

    void setupLanguageComboBox();
    void setupVoiceComboBox();
    void resetVoiceComboBox();
    void setupSpeedOptionsComboBox();
    void resetSpeedComboBox();

    int getVoiceIndex();

public slots:
    void onStateChanged(QTextToSpeech::State state);
    void speechClose();
    void speechShow();
    void toggleVoice();
    void toggleLanguage();
    void languageSelected(int index);
    void voiceSelected(int index);
    void onSpeedChanged(int index);
    void increaseSpeed();
    void decreaseSpeed();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTextToSpeech m_speech;
    Ui::TextToSpeechBar *ui;
    QVector<QVoice> m_voices;
    QString m_text;
};

#endif // TEXTTOSPEECHMANAGER_H
