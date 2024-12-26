<a href="https://flathub.org/apps/details/org.kiwix.desktop">
<img width="100" alt="Download on Flathub" src="https://flathub.org/assets/badges/flathub-badge-en.png" />
</a>

Kiwix Linux/Windows
=====================

The Kiwix Linux/Windows is a viewer/manager of ZIM files for GNU/Linux
and Microsoft Windows OSes.

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

The Kiwix application relies on many third party software libraries.
The following libraries need to be available:

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

Install needed packages (on Ubuntu 22.04+):

```bash
sudo apt install qt6-base-dev qt6-base-dev-tools qt6-webengine-dev \
     libqt6webenginecore6-bin libqt6svg6
```

Compilation
-----------

`qmake` needs to be configured to use Qt6:

```bash
qtchooser -install qt6 $(which qmake6)   # run once
export QT_SELECT=qt6                     # set in environments where Qt6 builds are desired
```

First confirm `qmake` is using the right version, this should produces this output:

```bash
$ qmake --version
QMake version 3.1
Using Qt version 6.2.4 in /usr/lib/aarch64-linux-gnu
```

Once all dependencies are installed, you can compile Kiwix:

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

You may want to simply open the `kiwix-desktop` project in QtCreator and
then compile the project from there (don't forget to update
`PKG_CONFIG_PATH` if necessary).

Compilation with Qt5
--------------------

It is still possible to compile using Qt5, following packages are needed:

```bash
sudo apt-get install libqt5gui5 qtbase5-dev qtwebengine5-dev \
     libqt5svg5-dev qt5-image-formats-plugins aria2 \
     qttools5-dev-tools qtchooser qt5-qmake \
     qtbase5-dev-tools
```

`qmake` needs to be configured to use Qt5:

```bash
qtchooser -install qt5 $(which qmake5)   # run once
export QT_SELECT=qt5                     # set in environments where Qt5 builds are desired
qmake --version
```

then build as normal:

```bash
qmake .
make
```

Compilation on Microsoft Windows
--------------------------------

Here is the [online
documentation](https://github.com/kiwix/kiwix-build/wiki/Compile-on-Microsoft-Windows).

Installation
------------

To install Kiwix on the system:
```bash
sudo make install
```

Run
---

To run Kiwix:
```bash
kiwix
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

## Communication

Available communication channels:
* [Web Public Chat channel](https://chat.kiwix.org)
* [Email](mailto:contact+android@kiwix.org)
* [Mailing list](mailto:kiwix-developer@lists.sourceforge.net)
* [Slack](https://kiwixoffline.slack.com): #android channel [Get an invite](https://join.slack.com/t/kiwixoffline/shared_invite/zt-19s7tsi68-xlgHdmDr5c6MJ7uFmJuBkg)
* IRC: #kiwix on irc.freenode.net

For more information, please refer to
[https://wiki.kiwix.org/wiki/Communication](https://wiki.kiwix.org/wiki/Communication).

License
-------

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[LICENSE](LICENSE) for more details.
