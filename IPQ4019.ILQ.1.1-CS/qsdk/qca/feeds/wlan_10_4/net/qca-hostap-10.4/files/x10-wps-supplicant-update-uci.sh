#!/bin/sh
#
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

IFNAME=$1
WPA_SUPPLICANT_CONF="/var/run/wpa_supplicant-$IFNAME.conf"
ANOTHER_IFNAME=""
CMD=$2

WPS_ALLOW_5G_FILE="/tmp/WPS_ON_5G"
WPS_ALLOW_2G_FILE="/tmp/WPS_ON_2G"
REMOVE_NETWORK_LIST_FILE="/tmp/remove_network_list"

if [ "$CMD" != "WPS-SUCCESS" ]; then
	exit 0
fi

update_wpa_supplicant_config() {
	# IFNAME
	wpa_cli -i$IFNAME -p/var/run/wpa_supplicant-$IFNAME save_config
	wpa_cli -i$IFNAME -p/var/run/wpa_supplicant-$IFNAME list_networks | sed '1d' | awk '{ print $1, $NF }' | grep "DISABLED" | awk '{ print $1 }' > $REMOVE_NETWORK_LIST_FILE
	for i in $(seq 1 300)
	do
		NETWORK_ID=`sed "$i!d" "$REMOVE_NETWORK_LIST_FILE"`
		if [ -n "$NETWORK_ID" ]; then
			wpa_cli -i$IFNAME -p/var/run/wpa_supplicant-$IFNAME remove_network $NETWORK_ID
		else
			break;
		fi
	done
	rm $REMOVE_NETWORK_LIST_FILE
	wpa_cli -i$IFNAME -p/var/run/wpa_supplicant-$IFNAME save_config

	# ANOTHER_IFNAME
	NETWORK_ID=$(wpa_cli -i$ANOTHER_IFNAME -p/var/run/wpa_supplicant-$ANOTHER_IFNAME list_networks | sed '1d' | awk '{ print $1}')
	wpa_cli -i$ANOTHER_IFNAME -p/var/run/wpa_supplicant-$ANOTHER_IFNAME set_network $NETWORK_ID ssid ""
	wpa_cli -i$ANOTHER_IFNAME -p/var/run/wpa_supplicant-$ANOTHER_IFNAME set_network $NETWORK_ID disabled 1
	wpa_cli -i$ANOTHER_IFNAME -p/var/run/wpa_supplicant-$ANOTHER_IFNAME save_config
	wpa_cli -i$ANOTHER_IFNAME -p/var/run/wpa_supplicant-$ANOTHER_IFNAME reconfigure
}

update_hostapd_config() {
	local new_ssid=$1
	local new_auth=$2
	local new_encr=$3
	local new_keys=
	local ssid_suffix_2G="_RPT"
	local ssid_suffix_5G="_RPT_5G"

	if [ "${new_auth}" = "WPA2PSK" ]; then
		new_keys=$4
	fi

	hostapd_cli -iath0 -p/var/run/hostapd-wifi0 wps_config "${new_ssid}${ssid_suffix_2G}" ${new_auth} ${new_encr} ${new_keys}
	hostapd_cli -iath1 -p/var/run/hostapd-wifi1 wps_config "${new_ssid}${ssid_suffix_5G}" ${new_auth} ${new_encr} ${new_keys}
}

op_mode=$(uci get system.@system[0].op_mode)
if [ "$op_mode" == "3" ]; then
	if [ "$IFNAME" == "ath0" ]; then
		if [ ! -f $WPS_ALLOW_2G_FILE ]; then
			exit 0
		else
			rm $WPS_ALLOW_2G_FILE
		fi
		ANOTHER_IFNAME="ath1"
	else
		if [ ! -f $WPS_ALLOW_5G_FILE ]; then
			exit 0
		else
			rm $WPS_ALLOW_5G_FILE
		fi
		ANOTHER_IFNAME="ath0"
	fi
elif [ "$op_mode" == "1" ]; then
	if [ "$IFNAME" == "ath01" ]; then
		if [ ! -f $WPS_ALLOW_2G_FILE ]; then
			exit 0
		else
			rm $WPS_ALLOW_2G_FILE
		fi
		ANOTHER_IFNAME="ath11"
	else
		if [ ! -f $WPS_ALLOW_5G_FILE ]; then
			exit 0
		else
			rm $WPS_ALLOW_5G_FILE
		fi
		ANOTHER_IFNAME="ath01"
	fi
fi

update_wpa_supplicant_config
ssid=$(awk 'BEGIN{FS="="} /ssid=/ {print $0}' $WPA_SUPPLICANT_CONF |grep "ssid=" |tail -n 1 | cut -f 2 -d= | sed -e 's/^"\(.*\)"/\1/')
key_mgmt=$(awk 'BEGIN{FS="="} /key_mgmt=/ {print $0}' $WPA_SUPPLICANT_CONF |grep "key_mgmt=" |tail -n 1 | cut -f 2 -d= | sed -e 's/^"\(.*\)"/\1/')
psk=$(awk 'BEGIN{FS="="} /psk=/ {print $0}' $WPA_SUPPLICANT_CONF |grep "psk=" |tail -n 1 | cut -f 2 -d= | sed -e 's/^"\(.*\)"/\1/')
if [ "$key_mgmt" == "WPA-PSK" ]; then
	uci set wireless.$IFNAME.encryption='psk2+aes'
	uci set wireless.$IFNAME.key=$psk
	if [ "$op_mode" == "1" ]; then
		update_hostapd_config "$ssid" WPA2PSK CCMP $psk
	fi
elif [ "$key_mgmt" == "NONE" ]; then
	uci set wireless.$IFNAME.encryption='none'
	uci set wireless.$IFNAME.key=''
	if [ "$op_mode" == "1" ]; then
		update_hostapd_config "$ssid" OPEN NONE
	fi
fi
uci set wireless.$IFNAME.ssid="$ssid"
uci set wireless.$ANOTHER_IFNAME.ssid=""
uci commit

kill "$(cat "/var/run/wps-supplicant-hotplug.pid")"
rm /var/run/wps-supplicant-hotplug.pid
kill "$(cat "/var/run/wps-hotplug-$IFNAME.pid")"
rm /var/run/wps-hotplug-$IFNAME.pid

x10_leds_ctl AMBER_LED 1001
sleep 5
x10_leds_ctl AMBER_LED 1000

rm /var/run/wps_is_running
