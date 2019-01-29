#!/bin/sh
if [ ! -f /tmp/sysinfo/fw_version ]; then # && [ ! -f /tmp/sysinfo/znp_version ]; then
    echo "system not ready.please retry later"
    exit 0
fi
MODEL=`boardcfg -g MODEL`
HW_VER=`boardcfg -g HW_VER`
WLAN_REG=`boardcfg -g WLAN_REG`
WPA=`boardcfg -g WPA`
SID=`boardcfg -g SID`
SW_VERSION=`cat /tmp/sysinfo/fw_version`
ZNP_VERSION=`cat /tmp/sysinfo/znp_version`
ZIGBEE_MAC=`cat /tmp/sysinfo/zigbee_mac`
WPS_PIN=`boardcfg -g WPS_PIN`
IR_VER=`ir_cmd -ver | grep version | cut -d'=' -f2`
SN=`boardcfg -g SN`
LORA=`test_lora -LoraGetMyAddr  | grep LoraMyAddr | cut -d'=' -f2`
LORA_VER=`test_lora -GetFWVersion  | grep FirmwareVersion | cut -d'=' -f2`
TELNET_ENABLE=`(ls -al /etc/rc.d/*telnet* | grep telnet > /dev/null && echo "enabled") || echo "diabled"`
FACTORY_IMAGE=`(test -e /usr/sbin/x10-diag && echo "YES") || echo "NO"`
CAL_STATUS=`(dmesg | grep qc98xx_verify_checksum) > /dev/null && (dmesg | grep qc98xx_verify_checksum | cut -d':' -f 2-3 | tail -n 2) || echo "flash checksum failed:"`
echo "FACTORY_IMAGE=$FACTORY_IMAGE"
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
A=`ifconfig wifi0 | grep HWaddr | cut -d' ' -f10 | cut -c1-17`;echo "[ WiFi0][  $A  ]"
A=`ifconfig wifi1 | grep HWaddr | cut -d' ' -f10 | cut -c1-17`;echo "[ WiFi1][  $A  ]"
A=`ifconfig eth0 | grep HWaddr | cut -d' ' -f11 | cut -c1-17`;echo "[ Eth0][  $A  ]"
A=`ifconfig eth1 | grep HWaddr | cut -d' ' -f11 | cut -c1-17`;echo "[ Eth1][  $A  ]"
