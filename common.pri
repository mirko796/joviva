QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SRC_DIR = $$PWD/src
INCLUDEPATH *= $$SRC_DIR
SOURCES *= \
    $$PWD/src/slundoredo.cpp \
    $$SRC_DIR/slcommon.cpp \
    $$SRC_DIR/slgraphicsitem.cpp \
    $$SRC_DIR/slgraphicspixmapitem.cpp \
    $$SRC_DIR/slgraphicsscene.cpp \
    $$SRC_DIR/slgraphicstextitem.cpp \
    $$SRC_DIR/slgraphicsview.cpp \
    $$SRC_DIR/slimageprovider.cpp \
    $$SRC_DIR/slmainwindow.cpp \
    $$SRC_DIR/slprintpreview.cpp \
    $$SRC_DIR/slsidebarwidget.cpp

HEADERS *= \
    $$PWD/src/slundoredo.h \
    $$SRC_DIR/slcommon.h \
    $$SRC_DIR/slgraphicsitem.h \
    $$SRC_DIR/slgraphicspixmapitem.h \
    $$SRC_DIR/slgraphicsscene.h \
    $$SRC_DIR/slgraphicstextitem.h \
    $$SRC_DIR/slgraphicsview.h \
    $$SRC_DIR/slimageprovider.h \
    $$SRC_DIR/slmainwindow.h \
    $$SRC_DIR/slprintpreview.h \
    $$SRC_DIR/slsidebarwidget.h

FORMS *= \
    $$SRC_DIR/slmainwindow.ui \
    $$SRC_DIR/slsidebarwidget.ui

RESOURCES *= \
    $$PWD/rsrc/images.qrc \
    $$PWD/translations/translations.qrc

