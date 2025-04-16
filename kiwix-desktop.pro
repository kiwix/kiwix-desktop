#-------------------------------------------------
#
# Project created by QtCreator 2018-04-11T15:26:46
#
#-------------------------------------------------

QT       += core gui network
QT       += webenginewidgets webchannel
QT       += printsupport
qtHaveModule(texttospeech): QT += texttospeech

win32 {
    QT += gui-private
}

# Avoid stripping incompatible files, due to false identification as executables, on WSL
DETECT_WSL = $$system(test -f /proc/sys/fs/binfmt_misc/WSLInterop && echo true || echo false)
equals(DETECT_WSL , "true"): CONFIG += nostrip
CONFIG += link_pkgconfig

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kiwix-desktop
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS +=  -std=c++17

!win32 {
    QMAKE_CXXFLAGS += -Werror
    equals(QT_MAJOR_VERSION, 6):equals(QT_MINOR_VERSION, 6) {
        # Fail the build on errors, except for 'template-id-cdtor' due to a problem with Qt headers.
        # This can be removed when the Ubuntu package is fixed.
        # See
        # https://github.com/RfidResearchGroup/proxmark3/issues/2382
        # https://bugreports.qt.io/browse/QTBUG-126989
        QMAKE_CXXFLAGS += -Wno-error=template-id-cdtor
    }
}

!win32:!static {
    QMAKE_LFLAGS += -Wl,-rpath-link,\'$$PREFIX/lib/x86_64-linux-gnu\'
}

# Also change resources/org.kiwix.desktop.appdata.xml
DEFINES += VERSION="2.4.1"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/choiceitem.cpp \
    src/contentmanagerdelegate.cpp \
    src/contentmanagerheader.cpp \
    src/contentmanagermodel.cpp \
    src/contenttypefilter.cpp \
    src/descriptionnode.cpp \
    src/downloadmanagement.cpp \
    src/findinpagebar.cpp \
    src/flowlayout.cpp \
    src/kiwixchoicebox.cpp \
    src/kiwixmessagebox.cpp \
    src/kiwixlineedit.cpp \
    src/kiwixlistwidget.cpp \
    src/kiwixloader.cpp \
    src/rownode.cpp \
    src/suggestionlistworker.cpp \
    src/suggestionlistmodel.cpp \
    src/suggestionlistdelegate.cpp \
    src/thumbnaildownloader.cpp \
    src/translation.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/kiwixapp.cpp \
    src/kprofile.cpp \
    src/blobbuffer.cpp \
    src/library.cpp \
    src/settingsmanager.cpp \
    src/settingsview.cpp \
    src/topwidget.cpp \
    src/urlschemehandler.cpp \
    src/webview.cpp \
    src/searchbar.cpp \
    src/mainmenu.cpp \
    src/webpage.cpp \
    src/about.cpp \
    src/contentmanager.cpp \
    src/contentmanagerview.cpp \
    src/tabbar.cpp \
    src/contentmanagerside.cpp \
    src/readinglistbar.cpp \
    src/tableofcontentbar.cpp \
    src/klistwidgetitem.cpp \
    src/opdsrequestmanager.cpp \
    src/localkiwixserver.cpp \
    src/fullscreenwindow.cpp \
    src/fullscreennotification.cpp \
    src/zimview.cpp \
    src/multizimbutton.cpp \
    src/versionchecker.cpp \

qtHaveModule(texttospeech): SOURCES += src/texttospeechbar.cpp \

HEADERS += \
    src/choiceitem.h \
    src/contentmanagerdelegate.h \
    src/contentmanagerheader.h \
    src/contentmanagermodel.h \
    src/contentmanagerview.h \
    src/contenttypefilter.h \
    src/descriptionnode.h \
    src/downloadmanagement.h \
    src/findinpagebar.h \
    src/flowlayout.h \
    src/kiwixchoicebox.h \
    src/kiwixmessagebox.h \
    src/kiwixlineedit.h \
    src/kiwixlistwidget.h \
    src/kiwixloader.h \
    src/node.h \
    src/rownode.h \
    src/suggestionlistworker.h \
    src/suggestionlistmodel.h \
    src/suggestionlistdelegate.h \
    src/thumbnaildownloader.h \
    src/translation.h \
    src/mainwindow.h \
    src/kiwixapp.h \
    src/kprofile.h \
    src/blobbuffer.h \
    src/library.h \
    src/settingsmanager.h \
    src/settingsview.h \
    src/topwidget.h \
    src/kconstants.h \
    src/urlschemehandler.h \
    src/webview.h \
    src/searchbar.h \
    src/mainmenu.h \
    src/webpage.h \
    src/about.h \
    src/contentmanager.h \
    src/tabbar.h \
    src/contentmanagerside.h \
    src/readinglistbar.h \
    src/tableofcontentbar.h \
    src/klistwidgetitem.h \
    src/opdsrequestmanager.h \
    src/localkiwixserver.h \
    src/fullscreenwindow.h \
    src/fullscreennotification.h \
    src/menuproxystyle.h \
    src/zimview.h \
    src/portutils.h \
    src/css_constants.h \
    src/multizimbutton.h \
    src/kiwixwebchannelobject.h \
    src/versionchecker.h \

qtHaveModule(texttospeech): HEADERS += src/texttospeechbar.h \

FORMS += \
    src/choiceitem.ui \
    src/contentmanagerview.ui \
    src/findinpagebar.ui \
    ui/kiwixchoicebox.ui \
    ui/kiwixmessagebox.ui \
    ui/mainwindow.ui \
    ui/about.ui \
    src/contentmanagerside.ui \
    src/readinglistbar.ui \
    ui/localkiwixserver.ui \
    ui/settings.ui \
    src/tableofcontentbar.ui \

qtHaveModule(texttospeech): FORMS += src/texttospeechbar.ui \

include(subprojects/QtSingleApplication/src/qtsingleapplication.pri)
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
  desktop_file.path = $$PREFIX/share/applications/
  desktop_file.files = resources/org.kiwix.desktop.desktop
  INSTALLS += desktop_file

  metainfo_file.path = $$PREFIX/share/metainfo/
  metainfo_file.files = resources/org.kiwix.desktop.appdata.xml
  INSTALLS += metainfo_file

  app_icon24.path = $$PREFIX/share/icons/hicolor/24x24/apps
  app_icon24.files = resources/icons/kiwix/24/kiwix-desktop.png
  app_icon32.path = $$PREFIX/share/icons/hicolor/32x32/apps
  app_icon32.files = resources/icons/kiwix/32/kiwix-desktop.png
  app_icon48.path = $$PREFIX/share/icons/hicolor/48x48/apps
  app_icon48.files = resources/icons/kiwix/48/kiwix-desktop.png
  app_icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps
  app_icon128.files = resources/icons/kiwix/128/kiwix-desktop.png
  app_icon256.path = $$PREFIX/share/icons/hicolor/256x256/apps
  app_icon256.files = resources/icons/kiwix/256/kiwix-desktop.png
  app_icon512.path = $$PREFIX/share/icons/hicolor/512x512/apps
  app_icon512.files = resources/icons/kiwix/512/kiwix-desktop.png
  app_svg.path = $$PREFIX/share/icons/hicolor/scalable/apps
  app_svg.files = resources/icons/kiwix/scalable/kiwix-desktop.svg
  INSTALLS += app_icon24 app_icon32 app_icon48 app_icon128 app_icon256 app_icon512 app_svg

  mime_icon48.path = $$PREFIX/share/icons/hicolor/48x48/mimetypes
  mime_icon48.files = resources/icons/48/kiwix/org.kiwix.desktop.x-zim.png
  mime_icon128.path = $$PREFIX/share/icons/hicolor/128x128/mimetypes
  mime_icon128.files = resources/icons/128/kiwix/org.kiwix.desktop.x-zim.png
  mime_icon256.path = $$PREFIX/share/icons/hicolor/256x256/mimetypes
  mime_icon256.files = resources/icons/256/kiwix/org.kiwix.desktop.x-zim.png
  mime_icon512.path = $$PREFIX/share/icons/hicolor/512x512/mimetypes
  mime_icon512.files = resources/icons/512/kiwix/org.kiwix.desktop.x-zim.png
  INSTALLS += mime_icon48 mime_icon128 mime_icon256 mime_icon512

  mime_file.path = $$PREFIX/share/mime/packages/
  mime_file.files = resources/org.kiwix.desktop-mime.xml
  INSTALLS += mime_file
}

DEPS_DEFINITION = \"libkiwix >= 14.0.0 libkiwix < 15.0.0 libzim >= 9.0.0 libzim < 10.0.0\"

PKGCONFIG_CFLAGS = $$system(pkg-config --cflags $$PKGCONFIG_OPTION $$DEPS_DEFINITION)

QMAKE_CXXFLAGS += $$PKGCONFIG_CFLAGS
QMAKE_CFLAGS += $$PKGCONFIG_CFLAGS

!win32 {
   LIBS += $$system(pkg-config --libs $$PKGCONFIG_OPTION $$DEPS_DEFINITION)
}

win32 {
  LIBS += $$system(python scripts/pkg-config-wrapper.py --libs $$PKGCONFIG_OPTION $$DEPS_DEFINITION)
}

RESOURCES += \
    resources/kiwix.qrc \
    resources/translations.qrc \
    resources/contentmanager.qrc \
    resources/settingsmanager.qrc \
    resources/style.qrc \
    resources/js.qrc

RC_ICONS = resources/icons/kiwix/app_icon.ico
