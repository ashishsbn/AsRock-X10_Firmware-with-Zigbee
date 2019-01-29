#!/bin/sh

URL=$1
FIRMWARE=/tmp/firmware.bin

wget -O $FIRMWARE $URL
md5=$(md5sum $FIRMWARE | awk '{ print $1 }')

if [ $2 == $md5 ]
then
	echo "md5 matches"
	sysupgrade -v $FIRMWARE
else
	echo "md5 not matches"
fi
