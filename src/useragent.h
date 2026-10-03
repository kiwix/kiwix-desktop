#ifndef USERAGENT_H
#define USERAGENT_H

#include <QString>

// Builds the User-Agent string identifying Kiwix desktop requests to
// Kiwix infrastructure, as requested in kiwix/kiwix-desktop#1521:
//   kiwix/<version> (desktop-linux)
//   kiwix/<version> (desktop-windows)
QString kiwixUserAgent();

#endif // USERAGENT_H
