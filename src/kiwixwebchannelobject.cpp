#include "kiwixwebchannelobject.h"
#include "kiwixapp.h"

QString KiwixWebChannelObject::getTocTitle() const
{
    return gt("table-of-content");
}

bool KiwixWebChannelObject::getTocVisible() const
{
    return KiwixApp::instance()->getAction(KiwixApp::ToggleTOCAction)->isChecked();
}
