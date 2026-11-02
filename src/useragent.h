#ifndef USERAGENT_H
#define USERAGENT_H

#include <QString>

// Builds the User-Agent string identifying Kiwix desktop requests to
// Kiwix infrastructure, as requested in kiwix/kiwix-desktop#1521:
//   kiwix/<version> (desktop-linux)
//   kiwix/<version> (desktop-windows)
QString kiwixUserAgent();

// Hands kiwixUserAgent() over to libkiwix (kiwix::setUserAgent()) so that the
// requests libkiwix makes on our behalf are identified as Kiwix Desktop too:
// book illustrations downloaded with curl, and downloads made through a
// kiwix::Downloader. Must be called before the first Downloader is created,
// because aria2c receives the User-Agent when it is launched.
void shareUserAgentWithLibkiwix();

#endif // USERAGENT_H
