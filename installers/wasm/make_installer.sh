#!/bin/sh
## define function to show error and exit
show_error() {
    echo "config.sh is missing"
    echo "Please create config.sh from config.sh.template and set the paths to reflect your system"
    exit 1
}
DESTDIR="../../docs"
if [ ! -d "$DESTDIR" ]; then
    echo "Dest dir not found, exiting..."
    exit 1
fi
# if config.sh is missing show error
[ ! -f config.sh ] && show_error 
# load config.sh
. ./config.sh
VERSION=`cat ../../version.txt`
DESTFILEPATH="$DESTDIR/$VERSION"
rm -rf "$DESTFILEPATH"
mkdir -p "$DESTFILEPATH"
echo "===== Building JovIva $DESTFILENAME ====="
echo "DESTDIR=$DESTDIR"
echo "DESTFILEPATH=$DESTFILEPATH"

echo "==== Config ===="
echo "QMAKE=$QMAKE"

echo "===== Clearing previous build ====="
rm -rf build
mkdir build
cd build
echo "===== Building WASM ====="
$QMAKE ../../../projects/JovIva/JovIva.pro CONFIG+=release -spec wasm-emscripten
make -j
echo "===== Copying to dest dir ====="
cd ..
cp ../out/* "$DESTFILEPATH"
rm "$DESTFILEPATH/JovIva.html"
ls -l "$DESTFILEPATH"