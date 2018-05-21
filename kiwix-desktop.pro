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
        main.cpp \
        mainwindow.cpp \
    kiwixschemehandler.cpp \
    kiwixapp.cpp \
    blobbuffer.cpp \
    kiwixrequestinterceptor.cpp \
    kiwixwebview.cpp

HEADERS += \
        mainwindow.h \
    kiwixschemehandler.h \
    kiwixapp.h \
    blobbuffer.h \
    kiwixrequestinterceptor.h \
    kiwixwebview.h

FORMS += \
        mainwindow.ui

isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin
INSTALLS += target

static {
  PKGCONFIG_OPTION = "--static"
  QMAKE_LFLAGS += "-static-libstdc++ --static"
}

unix {
  QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/../lib64\ '"
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
