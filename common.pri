# fetch version from ../../version.txt
VERSION = $$cat(../../version.txt)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
# get git commit hash
GIT_COMMIT = $$system(git rev-parse HEAD)
DEFINES += GIT_COMMIT=\\\"$$GIT_COMMIT\\\"


QT       += core gui printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SRC_DIR = $$PWD/src
INCLUDEPATH *= $$SRC_DIR
SOURCES *= \
    $$PWD/src/jipapersizedlg.cpp \
    $$SRC_DIR/jiaboutdlg.cpp \
    $$SRC_DIR/jicommon.cpp \
    $$SRC_DIR/jigraphicsitem.cpp \
    $$SRC_DIR/jigraphicspixmapitem.cpp \
    $$SRC_DIR/jigraphicsscene.cpp \
    $$SRC_DIR/jigraphicstextitem.cpp \
    $$SRC_DIR/jigraphicsview.cpp \
    $$SRC_DIR/jiimageprovider.cpp \
    $$SRC_DIR/jimainwindow.cpp \
    $$SRC_DIR/jiprintpreview.cpp \
    $$SRC_DIR/jisidebarwidget.cpp

HEADERS *= \
    $$PWD/src/jipapersizedlg.h \
    $$SRC_DIR/jiaboutdlg.h \
    $$SRC_DIR/jiundoredo.h \
    $$SRC_DIR/jicommon.h \
    $$SRC_DIR/jigraphicsitem.h \
    $$SRC_DIR/jigraphicspixmapitem.h \
    $$SRC_DIR/jigraphicsscene.h \
    $$SRC_DIR/jigraphicstextitem.h \
    $$SRC_DIR/jigraphicsview.h \
    $$SRC_DIR/jiimageprovider.h \
    $$SRC_DIR/jimainwindow.h \
    $$SRC_DIR/jiprintpreview.h \
    $$SRC_DIR/jisidebarwidget.h

FORMS *= \
    $$PWD/src/jipapersizedlg.ui \
    $$SRC_DIR/jiaboutdlg.ui \
    $$SRC_DIR/jimainwindow.ui \
    $$SRC_DIR/jisidebarwidget.ui

RESOURCES *= \
    $$PWD/rsrc/images.qrc \
    $$PWD/translations/translations.qrc

