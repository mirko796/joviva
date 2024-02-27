
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../common.pri)
SOURCES += \
    main.cpp

TRANSLATIONS += \
    ../../translations/srpski.ts
CONFIG += lrelease

ICON = ../../rsrc/app-icon.png
DESTDIR = ../../out
TARGET = JovIva
# Default rules for deployment.
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin
}

INSTALLS += target

wasm {
message("Wasm build");
# when build is done copy index.html from html to out folder
INDEX_HTML = $$PWD/../../html/index.html
message("Index html path" $$INDEX_HTML);
QMAKE_POST_LINK += $$quote(cp $$INDEX_HTML $$DESTDIR)
}
mac {
# build for both architectures
QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}
