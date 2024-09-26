#include "kiwixwebchannelobject.h"
#include "kiwixapp.h"

QString KiwixWebChannelObject::getTocTitle() const
{
    return gt("table-of-content");
}

QString KiwixWebChannelObject::getHideButtonText() const
{
    return gt("hide");
}

bool KiwixWebChannelObject::getTocVisible() const
{
    return KiwixApp::instance()->getAction(KiwixApp::ToggleTOCAction)->isChecked();
}

void KiwixWebChannelObject::setTocVisible(bool visible)
{
    if (getTocVisible() != visible)
        KiwixApp::instance()->getAction(KiwixApp::ToggleTOCAction)->toggle();
}
