
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../common.pri)
SOURCES += \
    main.cpp

TRANSLATIONS += \
    ../../translations/srpski.ts
CONFIG += lrelease
CONFIG += embed_translations
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
