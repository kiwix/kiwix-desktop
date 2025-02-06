#include "texttospeechbar.h"
#include "kiwixapp.h"
#include "ui_texttospeechbar.h"

TextToSpeechBar::TextToSpeechBar(QWidget *parent)
    : QFrame(parent), ui(new Ui::TextToSpeechBar)
{
    ui->setupUi(this);

    connect(&m_speech, &QTextToSpeech::stateChanged, this,
            &TextToSpeechBar::onStateChanged);

    ui->stopButton->setText(gt("stop"));
    ui->stopButton->setDisabled(true);

    const auto app = KiwixApp::instance();
    connect(app->getAction(KiwixApp::ReadStopAction), &QAction::triggered,
            this, &TextToSpeechBar::stop);
    connect(ui->stopButton, &QPushButton::pressed, this,
            &TextToSpeechBar::stop);
    connect(ui->closeButton, &QPushButton::pressed,
                this, &TextToSpeechBar::speechClose);

    setupSpeedOptionsComboBox();
    setupVoiceComboBox();
    setupLanguageComboBox();
    languageSelected(ui->langComboBox->currentIndex());
    connect(app->getAction(KiwixApp::ToggleTTSLanguageAction), &QAction::triggered,
            this, &TextToSpeechBar::toggleLanguage);
    connect(app->getAction(KiwixApp::ToggleTTSVoiceAction), &QAction::triggered,
            this, &TextToSpeechBar::toggleVoice);
    connect(app->getAction(KiwixApp::IncreaseTTSSpeedAction), &QAction::triggered,
            this, &TextToSpeechBar::increaseSpeed);
    connect(app->getAction(KiwixApp::DecreaseTTSSpeedAction), &QAction::triggered,
            this, &TextToSpeechBar::decreaseSpeed);
    connect(ui->speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TextToSpeechBar::onSpeedChanged);
}

void TextToSpeechBar::increaseSpeed()
{
    int currentIndex = ui->speedComboBox->currentIndex();
    if (currentIndex < ui->speedComboBox->count() - 1) {
        ui->speedComboBox->setCurrentIndex(currentIndex + 1);
    }
}

void TextToSpeechBar::decreaseSpeed()
{
    int currentIndex = ui->speedComboBox->currentIndex();
    if (currentIndex > 0) {
        ui->speedComboBox->setCurrentIndex(currentIndex - 1);
    }
}

void TextToSpeechBar::speak(const QString &text)
{
    m_text = text;
    m_speech.say(m_text);
}

void TextToSpeechBar::stop()
{
    m_speech.stop();
}

void TextToSpeechBar::setLocale(const QLocale& locale)
{
    for (int i = 0; i < ui->langComboBox->count(); i++)
    {
        if (ui->langComboBox->itemData(i).toLocale().language() == locale.language())
        {
            ui->langComboBox->setCurrentIndex(i);
            languageSelected(i);
            return;
        }
    }
}

void TextToSpeechBar::setupSpeedOptionsComboBox()
{
    ui->speedLabel->setText(gt("speed"));
    ui->speedComboBox->setMaxVisibleItems(10);
    ui->speedComboBox->setLineEdit(new ComboBoxLineEdit(ui->speedComboBox));

    QStringList speedOptions = {"0.25","0.50","0.75","1.00","1.25","1.50","1.75","2.00"};
    ui->speedComboBox->addItems(speedOptions);
}

void TextToSpeechBar::setupLanguageComboBox()
{
    ui->langLabel->setText(gt("language"));
    ui->langComboBox->setMaxVisibleItems(10);
    ui->langComboBox->setLineEdit(new ComboBoxLineEdit(ui->langComboBox));

    QLocale current = QLocale::system();
    for (const auto& locale : m_speech.availableLocales())
    {
        const QString name(QString("%1 (%2)")
                        .arg(QLocale::languageToString(locale.language()))
                        .arg(locale.nativeLanguageName()));

        ui->langComboBox->addItem(name, locale);
        if (locale.name() == current.name())
            current = locale;
    }

    ui->langComboBox->setCurrentIndex(ui->langComboBox->findData(current));
    connect(ui->langComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TextToSpeechBar::languageSelected);
}

void TextToSpeechBar::setupVoiceComboBox()
{
    ui->voiceLabel->setText(gt("voice"));
    ui->voiceComboBox->setMaxVisibleItems(10);
    ui->voiceComboBox->setLineEdit(new ComboBoxLineEdit(ui->voiceComboBox));
}

void TextToSpeechBar::resetVoiceComboBox()
{
    disconnect(ui->voiceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TextToSpeechBar::voiceSelected);
    ui->voiceComboBox->clear();

    m_voices = m_speech.availableVoices();
    if ( m_voices.isEmpty() )
	    return;

    for (const auto& voice : m_voices)
    {
        ui->voiceComboBox->addItem(QString("%1 - %2 - %3").arg(voice.name())
                          .arg(QVoice::genderName(voice.gender()))
                          .arg(QVoice::ageName(voice.age())));
    }

    const int voiceIndex = getVoiceIndex();
    ui->voiceComboBox->setCurrentIndex(voiceIndex);
    voiceSelected(voiceIndex);

    connect(ui->voiceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TextToSpeechBar::voiceSelected);
}

void TextToSpeechBar::resetSpeedComboBox()
{
    double savedSpeed = KiwixApp::instance()->getSavedTtsSpeed(m_speech.locale().name());
    int index = (savedSpeed - 0.25) * 4;
    ui->speedComboBox->setCurrentIndex(index);
}

int TextToSpeechBar::getVoiceIndex()
{
    int voiceIndex = 0;
    const QVoice currentVoice = m_speech.voice();
    const QString savedVoiceName = KiwixApp::instance()->getSavedVoiceName(m_speech.locale().name());

    /* We either stay with default voices or matches with the saved voice. */
    for (int i = 0; i < ui->voiceComboBox->count(); i++)
    {
        if (m_voices[i].name() == currentVoice.name())
            voiceIndex = i;
        if (m_voices[i].name() == savedVoiceName)
        {
            voiceIndex = i;
            break;
        }
    }
    return voiceIndex;
}

void TextToSpeechBar::speechClose()
{
    /* Prevent webview from scrolling to up to the top after losing focus. */
    const auto current = KiwixApp::instance()->getTabWidget()->currentWebView();
    if (!current)
        return;

    current->setFocus();
    m_speech.stop();
    close();
}

void TextToSpeechBar::speechShow()
{
    show();
    setFocus();
}

void TextToSpeechBar::toggleVoice()
{
    const auto zimView = KiwixApp::instance()->getTabWidget()->currentZimView();
    if (!zimView || zimView->getTextToSpeechBar() != this)
        return;

    if (isHidden())
        speechShow();

    ui->voiceComboBox->showPopup();
}

void TextToSpeechBar::toggleLanguage()
{
    const auto zimView = KiwixApp::instance()->getTabWidget()->currentZimView();
    if (!zimView || zimView->getTextToSpeechBar() != this)
        return;

    if (isHidden())
        speechShow();

    ui->langComboBox->showPopup();
}

void TextToSpeechBar::languageSelected(int index)
{
    const QLocale locale = ui->langComboBox->itemData(index).toLocale();
    m_speech.setLocale(locale);
    resetVoiceComboBox();
    resetSpeedComboBox();
}

void TextToSpeechBar::voiceSelected(int index)
{
    const auto voice = m_voices.at(index);
    const auto currentLang = ui->langComboBox->currentData().toLocale().name();
    KiwixApp::instance()->saveVoiceName(currentLang, voice.name());

    m_speech.setVoice(voice);
    if (m_speech.state() == QTextToSpeech::Speaking)
        speak(m_text);
}

void TextToSpeechBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        speechClose();
        return;
    }
    
    QFrame::keyPressEvent(event);
}

void TextToSpeechBar::onStateChanged(QTextToSpeech::State state)
{
    ui->stopButton->setEnabled(state != QTextToSpeech::Ready);
}

void TextToSpeechBar::onSpeedChanged(int index)
{
    QString speedText = ui->speedComboBox->itemText(index);
    double speed = speedText.toDouble();
    m_speech.setRate(speed - 1);  // range:-1,1

    // Save tts speed for current lang
    const auto currentLang = ui->langComboBox->currentData().toLocale().name();
    KiwixApp::instance()->saveTtsSpeed(currentLang, speed);

    // Restarting the speech with new speed set above
    if (m_speech.state() == QTextToSpeech::Speaking)
    {
        m_speech.stop();
        m_speech.say(m_text);
    }
}

ComboBoxLineEdit::ComboBoxLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setFrame(false);

    /* Work around to both have max visible item and a read-only combobox.*/
    setReadOnly(true);
    connect(this, &QLineEdit::selectionChanged, this, &ComboBoxLineEdit::preventSelection);
}

void ComboBoxLineEdit::preventSelection()
{
    setSelection(0, 0);
}

void ComboBoxLineEdit::mouseReleaseEvent(QMouseEvent *)
{
    const auto combo = qobject_cast<QComboBox*>(parent());
    if(combo)
        combo->showPopup();
}
