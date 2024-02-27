#!/bin/sh
## define function to show error and exit
show_error() {
    echo "config.sh is missing"
    echo "Please create config.sh from config.sh.template and set the paths to reflect your system"
    exit 1
}
DESTDIR="`pwd`/output"
mkdir -p "$DESTDIR"
if [ ! -d "$DESTDIR" ]; then
    echo "Dest dir not found, exiting..."
    exit 1
fi
# if config.sh is missing show error
[ ! -f config.sh ] && show_error 
# load config.sh
. ./config.sh
VERSION=`cat ../../version.txt`
DESTFILEPATH="$DESTDIR/joviva-mac-$VERSION.dmg"
MACDEPLOYQT="`dirname $QMAKE`/macdeployqt"
rm -rf "$DESTFILEPATH"
echo "===== Building JovIva $DESTFILENAME ====="
echo "DESTDIR=$DESTDIR"
echo "DESTFILEPATH=$DESTFILEPATH"

echo "==== Config ===="
echo "QMAKE=$QMAKE"
echo "MACDEPLOYQT=$MACDEPLOYQT"
echo "===== Clearing previous build ====="
rm -rf ../out
# rm -rf build
mkdir build
cd build
echo "===== Building Mac ====="
$QMAKE ../../../projects/JovIva/JovIva.pro CONFIG+=release -spec macx-clang
make -j
echo "===== Copying to dest dir ====="
cd ../../out
# run macdeployqt
$MACDEPLOYQT JovIva.app -dmg -verbose=2
ls -lh JovIva.dmg
mv JovIva.dmg "$DESTFILEPATH"
ls -lh "$DESTFILEPATH"