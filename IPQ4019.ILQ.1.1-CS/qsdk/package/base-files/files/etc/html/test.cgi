#!/bin/sh
echo "Content-Type: text/plain"
echo $'\r\n\r\n'
#off led
LED_OFF_ALL()
{
    echo "0" > /sys/class/gpio/gpio54/value
    echo "0" > /sys/class/gpio/gpio36/value
    echo "0" > /sys/class/gpio/gpio68/value
    echo "0" > /sys/class/gpio/gpio65/value
    echo "0" > /sys/class/gpio/gpio53/value
    echo "0" > /sys/class/gpio/gpio66/value
}

LED_ON_ALL()
{
    echo "1" > /sys/class/gpio/gpio54/value
    echo "1" > /sys/class/gpio/gpio36/value
    echo "1" > /sys/class/gpio/gpio68/value
    echo "1" > /sys/class/gpio/gpio65/value
    echo "1" > /sys/class/gpio/gpio53/value
    echo "1" > /sys/class/gpio/gpio66/value
}

#check password
password=`cat /etc/login | cut -d':' -f3`
if [ ! -f /var/logined ]; then
    echo "$QUERY_STRING" | grep "$password"
    if [ "$?" = "0" ]; then
	touch /var/factory
	touch /var/logined
	exit 0
    else
	echo "error"
	exit 0
    fi
fi
PARAM=$(echo $QUERY_STRING | tr -d ';:`"<>,./?!@#$%^&(){}[]')

if [ "$PARAM" = "ir=lean" ]; then
    echo "ir lean"
#    ir_cmd -r 1 | grep ""
elif [ "$PARAM" = "dut=info" ]; then
    MODEL=`boardcfg -g MODEL`
    HW_VER=`boardcfg -g HW_VER`
    WLAN_REG=`boardcfg -g WLAN_REG`
    WPA=`boardcfg -g WPA`
    SID=`boardcfg -g SID`
    SW_VERSION=`cat /tmp/sysinfo/fw_version`
    ZNP_VERSION=`cat /tmp/sysinfo/znp_version`
    ZIGBEE_MAC=`cat /tmp/sysinfo/zigbee_mac`
    WPS_PIN=`boardcfg -g WPS_PIN`
    IR_VER=`cat /tmp/sysinfo/ir_version`
    SN=`boardcfg -g SN`
    LORA_VER=`test_lora -GetFWVersion  | grep FirmwareVersion | cut -d'=' -f2`
    LORA=`test_lora -LoraGetMyAddr  | grep LoraMyAddr | cut -d'=' -f2`
    TELNET_ENABLE=`(ls -al /etc/rc.d/*telnet* | grep telnet > /dev/null && echo "enabled") || echo "diabled"`
    CAL_STATUS=`(dmesg | grep qc98xx_verify_checksum) > /dev/null && (dmesg | grep qc98xx_verify_checksum | cut -d':' -f 2-3 | tail -n 2) || echo "flash checksum failed:"`
    USB_SIDE=`test -e /sys/bus/usb/devices/3-1/manufacturer && cat /sys/bus/usb/devices/3-1/speed;`
    USB_SIDE_MFG=`test -e /sys/bus/usb/devices/3-1/manufacturer && cat /sys/bus/usb/devices/3-1/manufacturer`
    USB_SIDE_STATUS=`[ "$USB_SIDE" = "480" ] && echo "$USB_SIDE_MFG USB 2.0";[ ! -z "$USB_SIDE" ] && [ "$USB_SIDE" -ne "480" ] && echo "$USB_SIDE_MFG speed unknow"; [ -z "$USB_SIDE" ] && echo "empty"`
    USB_REAR=`test -e /sys/bus/usb/devices/2-1/manufacturer && cat /sys/bus/usb/devices/2-1/speed;`
    USB_REAR_MFG=`test -e /sys/bus/usb/devices/2-1/manufacturer && cat /sys/bus/usb/devices/2-1/manufacturer`
    if [ -z "$USB_REAR" ]; then
	USB_REAR=`test -e /sys/bus/usb/devices/1-1/manufacturer && cat /sys/bus/usb/devices/1-1/speed;`
	USB_REAR_MFG=`test -e /sys/bus/usb/devices/1-1/manufacturer && cat /sys/bus/usb/devices/1-1/manufacturer`
    fi
    USB_REAR_STATUS=`[ "$USB_REAR" = "480" ] && echo "$USB_REAR_MFG USB 2.0";[ "$USB_REAR" = "5000" ] && echo "$USB_REAR_MFG USB 3.0";[ ! -z "$USB_REAR" ] && [ "$USB_REAR" -ne "480" ] && [ "$USB_REAR" -ne "5000" ] && echo "$USB_REAR_MFG speed unknow $USB_REAR"; [ -z "$USB_REAR" ] && echo "empty"`

    WIFI_24G_SSID=`uci show wireless.ath0.ssid | cut -d'=' -f2`
    WIFI_5G_SSID=`uci show wireless.ath1.ssid | cut -d'=' -f2`		    

    if [ -f /usr/sbin/x10-diag ]; then
	echo "IMAGE_TYPE=Factory"
	echo "IMAGE_TYPE=Factory"
	echo "IMAGE_TYPE=Factory"
    else
	echo "IMAGE_TYPE=Final"
	echo "IMAGE_TYPE=Final"
	echo "IMAGE_TYPE=Final"
    fi
    echo "MODEL=$MODEL"
    echo "HW_VER=$HW_VER"
    echo "WLAN_REG=$WLAN_REG"
    echo "WPA=$WPA"
    echo "SID=$SID"
    echo "SW_VERSION=$SW_VERSION"
    echo "WPS_PIN=$WPS_PIN"
    echo "ZNP_VERSION=$ZNP_VERSION"
    echo "IR_VER=$IR_VER"
    echo "LORA_MAC=$LORA"
    echo "SN=$SN"
    echo "ZIGBEE_MAC=$ZIGBEE_MAC"
    echo "LORA_VER=$LORA_VER"
    echo "TELNET=$TELNET_ENABLE"
    echo "CAL_STATUS=$CAL_STATUS"
    echo "USB_SIDE=$USB_SIDE_STATUS"
    echo "USB_REAR=$USB_REAR_STATUS"
    echo "24G_SSID=$WIFI_24G_SSID"
    echo "5G_SSID=$WIFI_5G_SSID"	
    A=`ifconfig wifi0 | grep HWaddr | cut -d' ' -f10 | cut -c1-17`;echo "[ WiFi0][  $A  ]"
    A=`ifconfig wifi1 | grep HWaddr | cut -d' ' -f10 | cut -c1-17`;echo "[ WiFi1][  $A  ]"
    A=`ifconfig eth0 | grep HWaddr | cut -d' ' -f11 | cut -c1-17`;echo "[ Eth0][  $A  ]"
    A=`ifconfig eth1 | grep HWaddr | cut -d' ' -f11 | cut -c1-17`;echo "[ Eth1][  $A  ]"
elif [ "$PARAM" = "dut=dmz" ]; then
    echo "DMZ"
    #ifconfig eth0 192.168.2.1 netmask 255.255.255.0
    #route add default gw 192.168.2.20
    #uci set firewall.dmz.enabled=1
    #uci set firewall.dmz.dest_ip=192.168.1.30
    #uci commit firewall
    #/etc/init.d/firewall reload
    echo "DMZ done"
elif [ "$PARAM" = "dut=factoryreset" ]; then
    echo "factoryreset"
    #OUTPUT=`jffs2reset -y`
    #echo "$OUTPUT"
    #reboot
elif [ "$PARAM" = "dut=reboot" ]; then
    echo "reboot"
    #reboot
elif [ "$PARAM" = "dut=btnresettest" ]; then
    echo "btnresettest"
    if [ -f /var/btnresetpressed ]; then
	echo "Reset button is pressed"
    else
	echo "key error"
    fi
elif [ "$PARAM" = "dut=btnwpstest" ]; then
    echo "btnwpstest"
    if [ -f /var/btnwpspressed ]; then
	echo "Wps button is pressed"
    else
	echo "key error"
    fi
elif [ "$PARAM" = "dut=LED_ON_ALL" ]; then
    LED_ON_ALL
    ir_cmd -led 0 FF FF
    echo "LED_ON_ALL"
elif [ "$PARAM" = "dut=LED_OFF_ALL" ]; then
    LED_OFF_ALL
    ir_cmd -led 0 0 0
    echo "LED_OFF_ALL"
elif [ "$PARAM" = "dut=RED_LED" ]; then
    echo "LED_REDS"
    echo "1" > /sys/class/gpio/gpio54/value
    echo "1" > /sys/class/gpio/gpio36/value
elif [ "$PARAM" = "dut=BLUE_LEDS" ]; then
    echo "LED_BLUES"
    echo "1" > /sys/class/gpio/gpio68/value
    echo "1" > /sys/class/gpio/gpio52/value
elif [ "$PARAM" = "dut=RED_01_LED" ]; then
    echo "LED_RED_01"
    echo "1" > /sys/class/gpio/gpio54/value
elif [ "$PARAM" = "dut=RED_02_LED" ]; then
    echo "LED_RED_02"
    echo "1" > /sys/class/gpio/gpio36/value
elif [ "$PARAM" = "dut=BLUE_01_LED" ]; then
    echo "LED_BLUE_01"
    echo "1" > /sys/class/gpio/gpio68/value
elif [ "$PARAM" = "dut=LORA_LED" ]; then
    echo "LED_LORA"
    LED_OFF_ALL
    echo "1" > /sys/class/gpio/gpio65/value
elif [ "$PARAM" = "dut=ZIGBEE_LED" ]; then
    echo "LED_ZIGBEE"
    LED_OFF_ALL
    echo "1" > /sys/class/gpio/gpio66/value
elif [ "$PARAM" = "dut=AMBER_LED" ]; then
    echo "LED_AMBER"
    LED_OFF_ALL
    echo "1" > /sys/class/gpio/gpio53/value
elif [ "$PARAM" = "dut=I2C_RED_LED" ]; then
    echo "LED_RED"
    LED_OFF_ALL
    ir_cmd -led 0 FF 0
elif [ "$PARAM" = "dut=I2C_BLUE_LED" ]; then
    echo "LED_BLUE"
    LED_OFF_ALL
    ir_cmd -led 0 0 FF
elif [ "$PARAM" = "dut=rssi" ]; then
    echo "RSSI"
    WIFI0_0=`iwpriv ath0 txrx_fw_stats 3 && dmesg | tail -n 15 | grep "Chain 0"`
    WIFI0_1=`iwpriv ath0 txrx_fw_stats 3 && dmesg | tail -n 15 | grep "Chain 1"`
    WIFI1_0=`iwpriv ath1 txrx_fw_stats 3 && dmesg | tail -n 15 | grep "Chain 0"`
    WIFI1_1=`iwpriv ath1 txrx_fw_stats 3 && dmesg | tail -n 15 | grep "Chain 1"`
    echo "24G_CHAIN_0=$WIFI0_0"
    echo "24G_CHAIN_1=$WIFI0_1"
    echo "5G_CHAIN_0=$WIFI1_0"
    echo "5G_CHAIN_1=$WIFI1_1"
else
    echo "error"
fi
