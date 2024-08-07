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
    mp_ui->stopButton->setText(gt("stop"));
    mp_ui->voiceLabel->setText(gt("voice"));
    connect(mp_speech, &QTextToSpeech::stateChanged, this,
            &TextToSpeechBar::onStateChanged);
    connect(mp_ui->stopButton, &QPushButton::released, this,
            &TextToSpeechBar::stop);
    connect(mp_ui->closeButton, &QPushButton::released,
                this, &TextToSpeechBar::speechBarClose);

    mp_ui->langLabel->setText(gt("language"));
    mp_ui->langComboBox->setMaxVisibleItems(10);
    QLocale current = QLocale::system().language();
    for (auto locale : mp_speech->availableLocales())
    {
        QString name(QString("%1 (%2)")
                        .arg(QLocale::languageToString(locale.language()))
                        .arg(locale.nativeLanguageName()));
        QVariant localeVariant(locale);
        mp_ui->langComboBox->addItem(name, localeVariant);
        if (locale.name() == current.name())
            current = locale;
    }

    /* Work around to both have max visible item and a read-only combobox.*/
    mp_ui->langComboBox->lineEdit()->setReadOnly(true);
    mp_ui->langComboBox->lineEdit()->setFrame(false);

    connect(mp_ui->langComboBox, &QComboBox::currentIndexChanged,
                this, &TextToSpeechBar::languageSelected);
    mp_ui->langComboBox->setCurrentIndex(mp_ui->langComboBox->findData(current));
    languageSelected(mp_ui->langComboBox->currentIndex());
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

void TextToSpeechBar::languageSelected(int index)
{
    QLocale locale = mp_ui->langComboBox->itemData(index).toLocale();
    disconnect(mp_ui->voiceComboBox, &QComboBox::currentIndexChanged, this, &TextToSpeechBar::voiceSelected);
    mp_ui->voiceComboBox->clear();

    m_voices = mp_speech->availableVoices();
    QVoice currentVoice = mp_speech->voice();
    for (auto voice : m_voices)
    {
        mp_ui->voiceComboBox->addItem(QString("%1 - %2 - %3").arg(voice.name())
                          .arg(QVoice::genderName(voice.gender()))
                          .arg(QVoice::ageName(voice.age())));
        if (voice.name() == currentVoice.name())
            mp_ui->voiceComboBox->setCurrentIndex(mp_ui->voiceComboBox->count() - 1);
    }
    connect(mp_ui->voiceComboBox, &QComboBox::currentIndexChanged, this, &TextToSpeechBar::voiceSelected);
}

void TextToSpeechBar::voiceSelected(int index)
{
    mp_speech->setVoice(m_voices.at(index));
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
