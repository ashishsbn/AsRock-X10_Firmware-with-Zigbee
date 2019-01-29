#!/bin/sh
#
# Copyright (c) 2016 Pegatron corp.
#
# All Rights Reserved.
# Pegatron Confidential and Proprietary.
#
# For WPS Status

if [ "$ACTION" = "wps-success" ]; then
	echo "WPS-SUCCESS" > /tmp/wps_status
	# For WPS PBC, when one radio is success, need to disable another radio
	touch /tmp/wps_led
	if [ "$INTERFACE" = "ath0" ]; then
		hostapd_cli -p /var/run/hostapd-wifi1 -i ath1 wps_cancel
	else
		hostapd_cli -p /var/run/hostapd-wifi0 -i ath0 wps_cancel
	fi
	x10_leds_ctl AMBER_LED 1001
	sleep 5
	x10_leds_ctl AMBER_LED 1000
	rm -f /tmp/wps_led
fi

if [ "$ACTION" = "wps-timeout" ]; then
	echo "WPS-TIMEOUT" > /tmp/wps_status
	if [ -f /tmp/wps_cancel ]; then
		# Cancel by GUI -> Just STOP
		x10_leds_ctl AMBER_LED 1000
	else
		if [ ! -f /tmp/wps_led ]; then
			touch /tmp/wps_led
			x10_leds_ctl AMBER_LED 1
			sleep 10
			x10_leds_ctl AMBER_LED 1000
			rm -f /tmp/wps_led
		fi
	fi
fi
