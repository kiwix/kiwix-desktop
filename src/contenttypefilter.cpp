#include "kiwixapp.h"
#include "contenttypefilter.h"

ContentTypeFilter::ContentTypeFilter(QString name, QWidget *parent)
: QCheckBox(parent),
  m_name(name)
{
    setTristate(true);

    m_states[Qt::Unchecked] = gt("no-filter");
    m_states[Qt::PartiallyChecked] = gt("yes");
    m_states[Qt::Checked] = gt("no");
    setText(gt(m_name) + " : " + m_states[checkState()]);
    connect(this, &QCheckBox::stateChanged, this, &ContentTypeFilter::onStateChanged);
}

void ContentTypeFilter::onStateChanged(int state)
{
    setText(gt(m_name) + " : " + m_states[static_cast<Qt::CheckState>(state)]);
    setStyleSheet((state == 0) ? "" : "*{font-weight: bold}");
}
