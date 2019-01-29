#!/bin/sh

[ "$ACTION" = ifup ] || exit 0

#KKHuang: In repeater and bridge mode, do not restart dnsmasq whenever DUT is connected root AP
op_mode=$(uci_get system @system[0] op_mode)
if [ "$op_mode" == "1" -o "$op_mode" == "3" ]; then
	[ "$INTERFACE" = "lan" ] && exit 0
fi

/etc/init.d/dnsmasq enabled && /etc/init.d/dnsmasq start
