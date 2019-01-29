#!/bin/sh

cd `dirname $0`
QSDK_DIR="$PWD/qsdk"
BUILD_DIR="$PWD/build"

name=$(shuf -n1 /usr/share/dict/american-english-small | sed "s/'s//")
version=GSX-$(date "+%Y%m%d%H%M%S")-$name
echo $version > qsdk/package/base-files/files/etc/version
make -C $QSDK_DIR V=99
cp -af $QSDK_DIR/bin/ipq806x/openwrt* $BUILD_DIR/ipq

cd $BUILD_DIR
./gen_x10_image.sh
cd -

FIRMWARE="$BUILD_DIR/bin/$version"
cp "$BUILD_DIR/bin/pega_x10-nor-ipq40xx-single.img" "$FIRMWARE"
echo "Fimrware Image: $FIRMWARE"
