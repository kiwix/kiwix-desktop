<a href="https://flathub.org/apps/details/org.kiwix.desktop">
<img width="100" alt="Download on Flathub" src="https://flathub.org/assets/badges/flathub-badge-en.png" />
</a>

Kiwix Desktop
=============

Kiwix is an offline reader for Web content, primarily designed to make [Wikipedia](https://www.wikipedia.org/) available offline. It reads archives in the [ZIM](https://openzim.org) file format, a highly compressed open format with additional metadata. This is the Kiwix Desktop version - a viewer/manager of ZIM files for GNU/Linux (recent versions) and Microsoft Windows 10 & 11 OSes.

[![Release](https://img.shields.io/github/v/tag/kiwix/kiwix-desktop?label=release&sort=semver)](https://download.kiwix.org/release/kiwix-desktop/)
[![Repositories](https://img.shields.io/repology/repositories/kiwix-desktop?label=repositories)](https://github.com/kiwix/kiwix-desktop/wiki/Repology)
[![Build Status](https://github.com/kiwix/kiwix-desktop/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/kiwix/kiwix-desktop/actions?query=branch%3Amain)
[![CodeFactor](https://www.codefactor.io/repository/github/kiwix/kiwix-desktop/badge)](https://www.codefactor.io/repository/github/kiwix/kiwix-desktop)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix library compilation itself, we recommend to have a look
at [kiwix-build](https://github.com/kiwix/kiwix-build).

Dependencies
------------

The Kiwix Desktop application relies on many third party software libraries.
The following libraries need to be available:

* [libzim](https://github.com/openzim/libzim/)
* [libkiwix](https://github.com/kiwix/libkiwix/)
* [Qt](https://www.qt.io/)
* [aria2](https://aria2.github.io/)

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. The
compilation script will tell you if one of them is missing or too old.
In the worst case, you will have to download and compile a bleeding edge
version by hand.

Libkiwix has to be compiled dynamically, the best way to have it is
to use [kiwix-build](https://github.com/kiwix/kiwix-build).

Install needed packages (on Ubuntu 18.04+):

```bash
sudo apt-get install libqt5gui5 qtbase5-dev qtwebengine5-dev \
     libqt5svg5-dev qt5-image-formats-plugins aria2 \
     qttools5-dev-tools qtchooser qt5-qmake \
     qtbase5-dev-tools
```

Compilation
-----------

Once all dependencies are installed, you can compile Kiwix Desktop:

```bash
qmake .
make
```

`qmake` will use pkg-config to locate libraries. Depending on where
you've installed libkiwix (and other libraries) you may have to
update the env variable `PKG_CONFIG_PATH`. It can be set as follows,
for example, for x86-64 native systems:

```bash
export PKG_CONFIG_PATH="<...>/BUILD_native_dyn/INSTALL/lib/x86_64-linux-gnu/pkgconfig"
```

You may want to simply open the kiwix-desktop project in QtCreator and
then compile the project from there (don't forget to update
`PKG_CONFIG_PATH` if necessary).

Compilation with Qt6
--------------------

There is initial support for Qt6. Additional packages are needed:

```bash
sudo apt install qt6-base-dev qt6-base-dev-tools qt6-webengine-dev libqt6webenginecore6-bin libqt6svg6
```

And `qmake` needs to be configured to use Qt6. First confirm `qmake` is using the right version:

```bash
qtchooser -install qt6 $(which qmake6)   # run once
export QT_SELECT=qt6                     # set in environments where Qt6 builds are desired
qmake --version
```

produces this output:

```bash
$ qmake --version
QMake version 3.1
Using Qt version 6.2.4 in /usr/lib/aarch64-linux-gnu
```

then build as normal:

```bash
qmake .
make
```

Handling of 'template-id' related compilation errors
----------------------------------------------------

Many minor versions of Qt, for both Qt5 and Qt6, have difficulties to
compile because of 'template-id' related syntax errors. If your
compiler (`g++`) supports it, you can get rid of these errors by
telling the compiler to ignore them with the following command:

```bash
qmake QMAKE_CXXFLAGS="-Wno-error=template-id-cdtor" .
make
```

More info at:
* https://github.com/kiwix/kiwix-desktop/issues/1406
* https://bugzilla.redhat.com/show_bug.cgi?id=2280366
* https://github.com/RfidResearchGroup/proxmark3/issues/2382
* https://bugreports.qt.io/browse/QTBUG-126989

Compilation on Microsoft Windows
--------------------------------

Here is the [online
documentation](https://github.com/kiwix/kiwix-build/wiki/Compile-on-Microsoft-Windows).

Installation
------------
Windows users can install Kiwix Desktop using the prebuilt binaries
available at:
https://download.kiwix.org/release/kiwix-desktop/

To install Kiwix Desktop on the system:
```bash
sudo make install
```

Run
---

To run Kiwix Desktop
```bash
kiwix-desktop
```

You might have to refresh the `ld` database before:
```bash
sudo ldconfig
```

If you face problems such as `library not found...`, add the following
environment variable:

```bash
LD_LIBRARY_PATH="<...>/BUILD_native_dyn/INSTALL/lib/x86_64-linux-gnu"
```

Debug rendering of a ZIM
------------------------

`kiwix-desktop` uses [Qt
WebEngine](https://doc.qt.io/qt-6/qtwebengine-overview.html) to render
ZIM content, relying on a custom `zim:` protocol to expose the ZIM
data to the Web engine.

To debug issues related to WebEngine, follow these steps:

1. Launch `kiwix-desktop` with the environment variable `QTWEBENGINE_REMOTE_DEBUGGING=<port>` set (replace `<port>` with an available local port number).
2. Open the desired ZIM file in the `kiwix-desktop` user interface.
3. In a Chromium-based browser, go to: `http://localhost:<port>`.
4. This opens the remote debugging UI. Use it to access Chrome DevTools connected to the ZIM webpage rendered by WebEngine.

## Communication

Available communication channels:
* [Web Public Chat channel](https://chat.kiwix.org)
* [Email](mailto:contact+android@kiwix.org)
* [Mailing list](mailto:kiwix-developer@lists.sourceforge.net)
* [Slack](https://kiwixoffline.slack.com): #android channel [Get an invite](https://join.slack.com/t/kiwixoffline/shared_invite/zt-19s7tsi68-xlgHdmDr5c6MJ7uFmJuBkg)
* IRC: #kiwix on irc.freenode.net

For more information, please refer to
[https://wiki.kiwix.org/wiki/Communication](https://wiki.kiwix.org/wiki/Communication).

## Support
If you're enjoying using Kiwix, drop a ⭐️ on the repo!

License
-------

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[LICENSE](LICENSE) for more details.
