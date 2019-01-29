#!/bin/sh
#
# Copyright (c) 2016 Pegatron corp.
#
# All Rights Reserved.
# Pegatron Confidential and Proprietary.
#
# WPS process on STA
#

op_mode=$1
HOTPLUG_PID_FILE="/var/run/wps-supplicant-hotplug.pid"
WPS_ALLOW_5G_FILE="/tmp/WPS_ON_5G"
WPS_ALLOW_2G_FILE="/tmp/WPS_ON_2G"
WPS_RUNNING_FILE="/var/run/wps_is_running"

# Preparation
if [ "$op_mode" == "3" ]; then
WLAN_5G_IFACE="ath1"
WLAN_2G_IFACE="ath0"
elif [ "$op_mode" == "1" ]; then
WLAN_5G_IFACE="ath11"
WLAN_2G_IFACE="ath01"
fi
echo $$ > $HOTPLUG_PID_FILE
x10_leds_ctl AMBER_LED 2

# Disconnect as running WPS
wpa_cli -p "/var/run/wpa_supplicant-$WLAN_5G_IFACE" disconnect
wpa_cli -p "/var/run/wpa_supplicant-$WLAN_2G_IFACE" disconnect

# Run 5GHz WPS
touch $WPS_ALLOW_5G_FILE
dir="/var/run/wpa_supplicant-$WLAN_5G_IFACE"
wpa_cli -p "$dir" wps_pbc
pid=/var/run/wps-hotplug-${dir#"/var/run/wpa_supplicant-"}.pid
wpa_cli -p"$dir" -a/lib/wifi/wps-supplicant-update-uci -P$pid -B
sleep 60
# 5GHz WPS timeout
rm $WPS_ALLOW_5G_FILE
kill `cat $pid`
rm $pid
wpa_cli -p "$dir" wps_cancel
wpa_cli -p "$dir" disconnect

# Run 2GHz WPS
touch $WPS_ALLOW_2G_FILE
dir="/var/run/wpa_supplicant-$WLAN_2G_IFACE"
wpa_cli -p "$dir" wps_pbc
pid=/var/run/wps-hotplug-${dir#"/var/run/wpa_supplicant-"}.pid
wpa_cli -p"$dir" -a/lib/wifi/wps-supplicant-update-uci -P$pid -B
sleep 60
# 2GHz WPS timeout
rm $WPS_ALLOW_2G_FILE
kill `cat $pid`
rm $pid
wpa_cli -p "$dir" wps_cancel
wpa_cli -p "$dir" disconnect

# Recovery
wpa_cli -p "/var/run/wpa_supplicant-$WLAN_2G_IFACE" reconnect
wpa_cli -p "/var/run/wpa_supplicant-$WLAN_5G_IFACE" reconnect

x10_leds_ctl AMBER_LED 1
sleep 10
x10_leds_ctl AMBER_LED 1000

rm $WPS_RUNNING_FILE $HOTPLUG_PID_FILE
