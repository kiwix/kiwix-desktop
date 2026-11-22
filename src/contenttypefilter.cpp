#include "contenttypefilter.h"
#include "kiwixapp.h"

ContentTypeFilter::ContentTypeFilter(QString name, QWidget *parent)
: QCheckBox(parent),
  m_name(name)
{
    setTristate(true);

    m_states[Qt::Unchecked] = gt("no-filter");
    m_states[Qt::PartiallyChecked] = gt("yes");
    m_states[Qt::Checked] = gt("no");
    setText(gt(m_name) + " : " + m_states[checkState()]);
    const bool dark = KiwixApp::instance() && KiwixApp::instance()->isDarkTheme();
    setStyleSheet(dark ? "* { color: #9aa0a6; }" : "* { color: #666666; }");
    #if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(this, &QCheckBox::checkStateChanged, this, &ContentTypeFilter::onStateChanged);
    #else
        connect(this, &QCheckBox::stateChanged, this, &ContentTypeFilter::onStateChanged);
    #endif
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::themeChanged,
            this, [this](SettingsManager::Theme) { onStateChanged(static_cast<int>(checkState())); });
}

void ContentTypeFilter::onStateChanged(int state)
{
    setText(gt(m_name) + " : " + m_states[static_cast<Qt::CheckState>(state)]);
    const bool dark = KiwixApp::instance() && KiwixApp::instance()->isDarkTheme();
    setStyleSheet((state == 0) ? (dark ? "*{color: #9aa0a6;}" : "*{color: #666666;}")
                               : (dark ? "*{font-weight: bold; color: #f0f2f5;}"
                                       : "*{font-weight: bold; color: black;}"));
}
