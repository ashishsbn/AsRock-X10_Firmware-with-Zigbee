#!/bin/sh
SERVER_IP=192.168.1.30
ETH1_IP=192.168.1.1
IMG_NAME=img.bin
IMG_PATH=/tmp/tmp_img
if [ -f /var/skipupdate ] || [ -f /var/auto_update_lock ]; then
    exit 0
fi

if [ -f /etc/test_ip ]; then
    ETH1_IP=`cat /etc/test_ip | cut -d':' -f1`
    ifconfig br-lan $ETH1_IP
fi

if [ -f /etc/auto_update ] && [ ! -f /var/auto_update_lock ] && [ ! -f /usr/sbin/x10-diag ]; then
    x10_leds_ctl WAN_LED_STOP
    x10_leds_ctl AMBER_LED 2
    x10_leds_ctl LORA_LED 2
    x10_leds_ctl BLUE_LED 1000
    x10_leds_ctl RED_LED 1000
    exit 0
fi

if [ -f /etc/auto_update ] && [ ! -f /var/auto_update_lock ]; then
    touch /var/auto_update_lock
    SERVER_IP=`cat /etc/auto_update | cut -d':' -f1`
    IMG_NAME=`cat /etc/auto_update | cut -d':' -f2`
    echo "STOP WAN LED " > /dev/console
    x10_leds_ctl WAN_LED_STOP
    #check mtd10 is mount
    while true;
    do
	mount=`mount | grep mtdblock10`
	if [ "$mount" != "" ]; then
	    break
        fi
	sleep 1
    done

    while true;
    do
	#echo "0" > /sys/class/gpio/gpio68/value
	#echo "0" > /sys/class/gpio/gpio52/value
	#echo "1" > /sys/class/gpio/gpio54/value
	#echo "1" > /sys/class/gpio/gpio36/value
	ir_cmd -led 0 FF 0
	ping $SERVER_IP -c 1 -W 1
	if [ $? == 0 ]; then
	    break
	fi
	sleep 1
	ir_cmd -led 0 0 FF
	#echo "1" > /sys/class/gpio/gpio68/value
	#echo "1" > /sys/class/gpio/gpio52/value
	#echo "0" > /sys/class/gpio/gpio54/value
	#echo "0" > /sys/class/gpio/gpio36/value
	ping $SERVER_IP -c 1 -W 1
	if [ $? == 0 ]; then
	    break
	fi
	sleep 1
    done
else
    exit 0
fi

echo "1" > /sys/class/gpio/gpio66/value
echo "1" > /sys/class/gpio/gpio65/value
echo "1" > /sys/class/gpio/gpio53/value
echo "0" > /sys/class/gpio/gpio46/value
sleep 1
echo "1" > /sys/class/gpio/gpio46/value
ir_cmd -led 0 FF FF
sleep 4
#ir_cmd -led 0 0 0

if [ -f /etc/auto_update ]; then
    echo "start upgrade ....."
    (cd /var;wget -P /var http://$SERVER_IP/$IMG_NAME && wget -P /var http://$SERVER_IP/$IMG_NAME.md5 && md5sum -c /var/*.md5 && imgchk.sh /var/$IMG_NAME && sysupgrade $IMG_PATH) && RESULT="OK" || RESULT="FAIL"
    if [ "$RESULT" = "FAIL" ]; then
	#light on
	echo "update error" > /dev/console
	ir_cmd -led 0 FF 0
	#echo "1" > /sys/class/gpio/gpio68/value
	#echo "1" > /sys/class/gpio/gpio36/value
    else
	echo "update ok...." > /dev/console
	#ir_cmd -led 0 0 FF
	#x10_leds_ctl RED_LED 1000
	#x10_leds_ctl BLUE_LED 1001
	#echo "1" > /sys/class/gpio/gpio54/value
	#echo "1" > /sys/class/gpio/gpio36/value
    fi
fi
