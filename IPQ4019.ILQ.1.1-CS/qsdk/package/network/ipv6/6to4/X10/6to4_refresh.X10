# ASRock X10 6to4 Refresh script
#!/bin/sh

. /lib/config/uci.sh
. /usr/share/libubox/jshn.sh

# get wan type
local proto="$(uci_get network wan proto)"
local proto6="$(uci_get network wan6 proto)"

echo "WAN:${proto}, WAN6:${proto6}"

if [ "${proto}" == "pppoe" ] && [ "${proto6}" == "6to4" ]; then
	# get pppoe status
	json_load "$(devstatus pppoe-wan)"
	json_get_var pppoe_up up
	echo "${devname}, up:${pppoe_up}"

	if [ "${pppoe_up}" == "1" ]; then
		# renew wan6 interface, obliging 6to4 to refresh
		ubus call network.interface.wan6 remove
		sleep 2
		ubus call network reload
		#echo $(date) >> /var/log/6to4.log
	fi
fi