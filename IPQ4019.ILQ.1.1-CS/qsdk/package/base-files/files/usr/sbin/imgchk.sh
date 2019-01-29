#!/bin/sh
TMP_IMAGE=/tmp/tmp_img
IMAGE_PATH=$1
#echo "UPGRADE_IMAGE_PATH=$IMAGE_PATH"
if [ ! -f "$IMAGE_PATH" ]; then
    exit 1
fi
eval `imagecheck -c $IMAGE_PATH`
echo "$tag0" > /tmp/imgchklog
echo "$tag1" >> /tmp/imgchklog
echo "$tag2" >> /tmp/imgchklog
echo "$tag3" >> /tmp/imgchklog
echo "$tag4" >> /tmp/imgchklog

MODEL=ASRock`boardcfg -g MODEL`

# X10R & X10 share same image
if [ "$MODEL" == ASRockX10R ]; then
    MODEL=ASRockX10
fi

if [ "$MODEL" != "$tag0" ]; then
    echo "MODEL name not match"
    exit 1
fi

if [ "$VERIFY" == "FAILED" ]; then
    echo "image verify failed"
    exit 1
else
    echo "image verify passed"
    cp $TMP_IMAGE $IMAGE_PATH
    if [ $? != 0 ]; then
	echo "can't copy image"
	exit 1
    fi
fi
exit 0
