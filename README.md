Kiwix desktop
=============

The Kiwix-desktop is a view/manager of zim files for GNU/Linux and Windows.
You can download and view your zim files as you which.

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix libary compilation itself, we recommend to have a look
to [kiwix-build](https://github.com/kiwix/kiwix-build).

Dependencies
------------

The kiwix-desktop application relies on many third parts software libraries.
Following libraries need to be available:

* kiwix-lib ...................... https://github.com/kiwix/kiwix-lib/
* Qt .............................................. https://www.qt.io/
* aria2 ..................................... https://aria2.github.io/

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. The
compilation script will tell you if one of them is missing or too old.
In the worse case, you will have to download and compile bleeding edge
version by hand.

kiwix-lib has to be compiled dynamically, the best way to have it is
to use [kiwix-build](https://github.com/kiwix/kiwix-build).

Install needed packages (on Ubuntu):
```
$ sudo apt-get install libqt5gui qtbase5-dev qtwebengine5-dev libgt5svg5-dev qt5-image-formats-plugins qt5-default aria2
```

Compilation
-----------

Once all dependencies are installed, you can compile the kiwix-desktop
with:
```
qmake .
make
make install
```

`qmake` will use pkg-config to locate libraries. Depending of where you've
installed kiwix-lib (and other libraries) you may have to update the env
variable `PKG_CONFIG_PATH`.

You may want to simply open the kiwix-desktop project in QtCreator and compile
the project from here (We will may have to also update `PKG_CONFIG_PATH`).

License
-------

GPLv3 or later, see COPYING for more details.
