#include "useragent.h"

#include "kiwixapp.h"

#include <kiwix/tools.h>

QString kiwixUserAgent()
{
#ifdef Q_OS_WIN
    const char* const platform = "desktop-windows";
#else
    const char* const platform = "desktop-linux";
#endif
    return QStringLiteral("kiwix/%1 (%2)")
        .arg(version, QLatin1String(platform));
}

void shareUserAgentWithLibkiwix()
{
    kiwix::setUserAgent(kiwixUserAgent().toStdString());
}
