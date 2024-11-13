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
    connect(KiwixApp::instance()->getAction(KiwixApp::ReadStopAction), &QAction::triggered,
            this, &TextToSpeechBar::stop);
    connect(ui->stopButton, &QPushButton::pressed, this,
            &TextToSpeechBar::stop);
    connect(ui->closeButton, &QPushButton::pressed,
                this, &TextToSpeechBar::speechClose);

    setupLanguageComboBox();
    languageSelected(ui->langComboBox->currentIndex());
}

void TextToSpeechBar::speak(const QString &text)
{
    m_speech.say(text);
}

void TextToSpeechBar::stop()
{
    m_speech.stop();
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

void TextToSpeechBar::languageSelected(int index)
{
    const QLocale locale = ui->langComboBox->itemData(index).toLocale();
    m_speech.setLocale(locale);
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
