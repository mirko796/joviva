#!/bin/sh
## define function to show error and exit
show_error() {
    echo "config.sh is missing"
    echo "Please create config.sh from config.sh.template and set the paths to reflect your system"
    exit 1
}
mkdir -p output
# if config.sh is missing show error
[ ! -f config.sh ] && show_error 
# load config.sh
. ./config.sh
VERSION=`cat ../../version.txt`
# get system architecture
ARCH=`uname -m`
DESTFILENAME=JovIva-$ARCH-$VERSION.AppImage
TMPFILE=JovIva.AppImage
DESTDIR="`pwd`/output"
DESTFILEPATH="$DESTDIR/$DESTFILENAME"

echo "===== Building JovIva $DESTFILENAME ====="
echo "DESTDIR=$DESTDIR"
echo "DESTFILEPATH=$DESTFILEPATH"

echo "==== Config ===="
echo "QMAKE=$QMAKE"
echo "LINUXDEPLOY=$LINUXDEPLOY"
echo "APPIMAGETOOL=$APPIMAGETOOL"

echo "===== Clearing previous build ====="
rm -rf build
mkdir build
cd build
echo "===== Building ====="
$QMAKE ../../../projects/JovIva/JovIva.pro CONFIG+=release
make -j$(nproc)
make install INSTALL_ROOT=AppDir
echo "===== Deploying ====="
QMAKE=$QMAKE $LINUXDEPLOY --appdir AppDir -pqt -d../joviva.desktop -i../../../rsrc/app-icon.png
$APPIMAGETOOL AppDir $TMPFILE
echo "===== Moving to output ====="
mv $TMPFILE "$DESTFILEPATH"
echo "===== Done ====="
echo "Result:"
ls -l "$DESTFILEPATH"
