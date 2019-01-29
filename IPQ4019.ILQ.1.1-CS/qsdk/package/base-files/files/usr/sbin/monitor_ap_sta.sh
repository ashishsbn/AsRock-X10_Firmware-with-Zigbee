#!/bin/sh

. /lib/functions.sh

STA_CONNECTED=1
SSID_VISIBLE=1
RECONFIG=0
INVALID_BSSID="00:00:00:00:00:00"
SLEEP=60

get_wifi_iface_params()
{
	local cfg=$1

	config_get ssid $cfg ssid
	config_get mode $cfg mode

	vif=$(iwconfig 2>/dev/null | awk '/'"$ssid"'/{print $1}')
	bssid=$(iwinfo ath0 info | head -2 | tail -1 | sed -e 's/.*Access Point: //g')

	if [ x$mode == x"sta" ] && [ x"$bssid" == x"$INVALID_BSSID" ]; then
		iwlist $vif scan | grep -q $ssid
		if [ $? -ne 0 ]; then
			STA_CONNECTED=0
		else
			STA_CONNECTED=1
		fi
	fi
}

set_sta()
{
	local cfg=$1
	disabled=$2

	config_get old_disabled $cfg disabled
	config_get mode $cfg mode

	if [ x$mode == x"sta" ] && [ x"$old_disabled" != x"$disabled" ] ; then
		uci set wireless.$cfg.disabled=$disabled
		uci commit wireless
		wifi up
	fi  
}

disable_sta()
{
	set_sta $1 1
}

enable_sta()
{
	set_sta $1 0
}

sleep $((SLEEP * 2))

while [ 1 ]; do
	STA_CONNECTED=1
	SSID_VISIBLE=1
	RECONFIG=0

	config_load wireless
	config_foreach get_wifi_iface_params wifi-iface

	if [ x$STA_CONNECTED == x"0" ]; then 
		config_load wireless
		config_foreach disable_sta wifi-iface
	elif [ x$STA_CONNECTED == x"1" ]; then
		config_load wireless
		config_foreach enable_sta wifi-iface
	fi

	sleep $SLEEP
done
