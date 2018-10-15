#include "contentmanager.h"
#include "contentmanager.h"

#include <QDebug>

ContentManager::ContentManager(Library* library, QObject *parent)
    : QObject(parent),
      mp_library(library)
{
    // mp_view will be passed to the tab who will take ownership,
    // so, we don't need to delete it.
    mp_view = new ContentManagerView();
    mp_view->registerObject("contentManager", this);
    mp_view->registerObject("library", mp_library);
    mp_view->setHtml();
}
