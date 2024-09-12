#include "kiwixapp.h"
#include "texttospeechbar.h"
#include "ui_texttospeechbar.h"
#include <QDebug>

// QTextToSpeech TextToSpeechBar::m_speech;
TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent),
      mp_ui(new Ui::TextToSpeechBar)
{
    close();
    mp_ui->setupUi(this);
    mp_ui->stopButton->setText(gt("stop"));
    mp_ui->voiceLabel->setText(gt("voice"));
    connect(&m_speech, &QTextToSpeech::stateChanged, this,
            &TextToSpeechBar::onStateChanged);
    connect(mp_ui->stopButton, &QPushButton::released, this,
            &TextToSpeechBar::stop);
    connect(mp_ui->closeButton, &QPushButton::released,
                this, &TextToSpeechBar::speechBarClose);

    mp_ui->langLabel->setText(gt("language"));
    mp_ui->langComboBox->setMaxVisibleItems(10);
    mp_ui->langComboBox->setLineEdit(new ComboBoxLineEdit(mp_ui->langComboBox));
    connect( mp_ui->langComboBox->lineEdit(), &QLineEdit::selectionChanged, mp_ui->langComboBox->lineEdit(), [=]()
    {
        mp_ui->langComboBox->lineEdit()->setSelection(0,0);
    });
    
    mp_ui->voiceComboBox->setLineEdit(new ComboBoxLineEdit(mp_ui->langComboBox));
    connect( mp_ui->voiceComboBox->lineEdit(), &QLineEdit::selectionChanged, mp_ui->voiceComboBox->lineEdit(), [=]()
    {
        mp_ui->voiceComboBox->lineEdit()->setSelection(0,0);
    });
    QLocale current = QLocale::system().language();
    for (auto locale : m_speech.availableLocales())
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

    connect(mp_ui->langComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TextToSpeechBar::languageSelected);
    mp_ui->langComboBox->setCurrentIndex(mp_ui->langComboBox->findData(current));
    languageSelected(mp_ui->langComboBox->currentIndex());
}

void TextToSpeechBar::speak(const QString &text)
{
    m_text = text;
    m_speech.say(text);
}

void TextToSpeechBar::stop()
{
    m_speech.stop();
}

void TextToSpeechBar::setLocale(QLocale locale)
{
    for (int i = 0; i < mp_ui->langComboBox->count(); i++)
    {
        if (mp_ui->langComboBox->itemData(i).toLocale().language() == locale.language())
        {
            mp_ui->langComboBox->setCurrentIndex(i);
            languageSelected(i);
            return;
        }
    }
}

void TextToSpeechBar::onStateChanged(QTextToSpeech::State state)
{
    mp_ui->stopButton->setEnabled(state != QTextToSpeech::Ready);
}

void TextToSpeechBar::languageSelected(int index)
{
    QLocale locale = mp_ui->langComboBox->itemData(index).toLocale();
    disconnect(mp_ui->voiceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TextToSpeechBar::voiceSelected);
    mp_ui->voiceComboBox->clear();

    m_speech.setLocale(locale);
    m_voices = m_speech.availableVoices();
    mp_ui->voiceComboBox->setMaxVisibleItems(10);
    QVoice currentVoice = m_speech.voice();
    QString savedVoiceName = KiwixApp::instance()->getSavedVoiceName(locale.name());
    for (auto voice : m_voices)
    {
        mp_ui->voiceComboBox->addItem(QString("%1 - %2 - %3").arg(voice.name())
                          .arg(QVoice::genderName(voice.gender()))
                          .arg(QVoice::ageName(voice.age())));
    }

    int voiceIndex = 0;
    for (int i = 0; i < mp_ui->voiceComboBox->count(); i++)
    {
        if (m_voices[i].name() == currentVoice.name())
            voiceIndex = i;
        if (m_voices[i].name() == savedVoiceName)
        {
            voiceIndex = i;
            break;
        }
    }
    mp_ui->voiceComboBox->setCurrentIndex(voiceIndex);
    voiceSelected(voiceIndex);

    mp_ui->voiceComboBox->lineEdit()->setReadOnly(true);
    mp_ui->voiceComboBox->lineEdit()->setFrame(false);
    connect(mp_ui->voiceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TextToSpeechBar::voiceSelected);
}

void TextToSpeechBar::voiceSelected(int index)
{   
    auto voice = m_voices.at(index);
    auto currentLang = mp_ui->langComboBox->currentData().toLocale().name();
    KiwixApp::instance()->saveVoiceName(currentLang, voice.name());

    m_speech.setVoice(voice);
    if (m_speech.state() == QTextToSpeech::Speaking)
        speak(m_text);
}

void TextToSpeechBar::speechBarClose()
{
    /* Prevent webview from scrolling to up to the top after losing focus. */
    auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;
    current->setFocus();

    m_speech.stop();
    close();
}

void ComboBoxLineEdit::mouseReleaseEvent(QMouseEvent *)
{
    QComboBox* combo = dynamic_cast<QComboBox*>(parent());
    if(combo)
        combo->showPopup();
}
