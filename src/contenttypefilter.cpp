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
    setStyleSheet("* { color: #666666; }");
    #if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(this, &QCheckBox::checkStateChanged, this, &ContentTypeFilter::onStateChanged);
    #else
        connect(this, &QCheckBox::stateChanged, this, &ContentTypeFilter::onStateChanged);
    #endif
}

void ContentTypeFilter::onStateChanged(int state)
{
    setText(gt(m_name) + " : " + m_states[static_cast<Qt::CheckState>(state)]);
    setStyleSheet((state == 0) ? "*{color: #666666;}" : "*{font-weight: bold; color: black;}");
}
