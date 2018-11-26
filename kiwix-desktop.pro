#-------------------------------------------------
#
# Project created by QtCreator 2018-04-11T15:26:46
#
#-------------------------------------------------

QT       += core gui
QT       += webenginewidgets

CONFIG += link_pkgconfig

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kiwix-desktop
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS +=  -std=c++11

unix {
  DEFINES += GIT_VERSION='"$(shell cd $$PWD && git describe --dirty=* --tags --always)"'
  DEFINES += BUILD_DATE='"$(shell date)"'
}

win32 {
  DEFINES += GIT_VERSION='"$$system(cd $$PWD && git describe --dirty=* --tags --always)"'
  DEFINES += BUILD_DATE='"$$system(echo %DATE%)"'
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/kiwixapp.cpp \
    src/blobbuffer.cpp \
    src/library.cpp \
    src/topwidget.cpp \
    src/urlschemehandler.cpp \
    src/webview.cpp \
    src/searchbar.cpp \
    src/mainmenu.cpp \
    src/webpage.cpp \
    src/about.cpp \
    src/tocsidebar.cpp \
    src/contentmanager.cpp \
    src/contentmanagerview.cpp \
    src/tabbar.cpp \
    src/contentmanagerside.cpp

HEADERS += \
    src/mainwindow.h \
    src/kiwixapp.h \
    src/blobbuffer.h \
    src/library.h \
    src/topwidget.h \
    src/kconstants.h \
    src/urlschemehandler.h \
    src/webview.h \
    src/searchbar.h \
    src/mainmenu.h \
    src/webpage.h \
    src/about.h \
    src/tocsidebar.h \
    src/contentmanager.h \
    src/contentmanagerview.h \
    src/tabbar.h \
    src/contentmanagerside.h

FORMS += \
    ui/mainwindow.ui \
    ui/about.ui \
    src/tocsidebar.ui \
    src/contentmanagerside.ui

TRANSLATIONS = "resources/i18n/kiwix-desktop_fr.ts"
CODECFORSRC = UTF-8

isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin
INSTALLS += target

static {
  PKGCONFIG_OPTION = "--static"
}

unix {
  QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/../lib64\ '"

  desktop_file.path = $$PREFIX/share/applications/
  desktop_file.files = resources/kiwix-desktop.desktop
  INSTALLS += desktop_file

  icon_file24.path = $$PREFIX/share/icons/hicolor/24x24/apps
  icon_file24.files = resources/icons/kiwix/24/kiwix-desktop.png
  icon_file32.path = $$PREFIX/share/icons/hicolor/32x32/apps
  icon_file32.files = resources/icons/kiwix/32/kiwix-desktop.png
  icon_file48.path = $$PREFIX/share/icons/hicolor/48x48/apps
  icon_file48.files = resources/icons/kiwix/48/kiwix-desktop.png
  icon_file128.path = $$PREFIX/share/icons/hicolor/128x128/apps
  icon_file128.files = resources/icons/kiwix/128/kiwix-desktop.png
  icon_file256.path = $$PREFIX/share/icons/hicolor/256x256/apps
  icon_file256.files = resources/icons/kiwix/256/kiwix-desktop.png
  INSTALLS += icon_file24 icon_file32 icon_file48 icon_file128 icon_file256
}

PKGCONFIG_CFLAGS = $$system(pkg-config --cflags $$PKGCONFIG_OPTION kiwix)

PKGCONFIG_INCLUDEPATH = $$find(PKGCONFIG_CFLAGS, ^-I.*)
PKGCONFIG_INCLUDEPATH ~= s/^-I(.*)/\\1/g

PKGCONFIG_DEFINES = $$find(PKGCONFIG_CFLAGS, ^-D.*)
PKGCONFIG_DEFINES ~= s/^-D(.*)/\\1/g

PKGCONFIG_CFLAGS ~= s/^-[ID].*//g

INCLUDEPATH *= $$PKGCONFIG_INCLUDEPATH
DEFINES *= $$PKGCONFIG_DEFINES

QMAKE_CXXFLAGS += $$PKGCONFIG_CFLAGS
QMAKE_CFLAGS += $$PKGCONFIG_CFLAGS

LIBS += $$system(pkg-config --libs $$PKGCONFIG_OPTION kiwix)

RESOURCES += \
    resources/kiwix.qrc \
    resources/translations.qrc \
    resources/contentmanager.qrc \
    resources/style.qrc

unix {
    system($$QMAKE_LUPDATE -locations relative -no-ui-lines $$_PRO_FILE_)
    system($$QMAKE_LRELEASE $$_PRO_FILE_)
}
