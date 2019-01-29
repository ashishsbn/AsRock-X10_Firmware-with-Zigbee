#!/bin/sh
while true;
do
mount=`mount | grep mtdblock10`
if [ "$mount" != "" ]; then
    break
fi
sleep 1
done

jffs2reset -y


