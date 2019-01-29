# ASRock X10 xl2tpd settings
#!/bin/sh

. /lib/config/uci.sh

# get X10 IPSec information from config
local enabled=$(uci_get ipsec strongswan enabled)

# Copy configurations from booting
if [ $enabled -eq 1 ]; then
	echo "Copy l2tp/ipsec configurations..."
	cp /etc/ppp/options.xl2tpd.ipsec /etc/ppp/options.xl2tpd
	cp /etc/xl2tpd/xl2tpd.conf.ipsec /etc/xl2tpd/xl2tpd.conf

	# set firewall
	local ipsec_esp=$(uci show firewall | grep "IPSec ESP" | sed 's/^.*firewall.//g' | sed 's/.name.*$//g')
	local ipsec_ika=$(uci show firewall | grep "IPSec IKE" | sed 's/^.*firewall.//g' | sed 's/.name.*$//g')
	local ipsec_nat=$(uci show firewall | grep "IPSec NAT-T" | sed 's/^.*firewall.//g' | sed 's/.name.*$//g')
	local auth_header=$(uci show firewall | grep "Auth Header" | sed 's/^.*firewall.//g' | sed 's/.name.*$//g')

	local ipsec_esp_enabled=$(uci_get firewall ${ipsec_esp} enabled)
	if [ $ipsec_esp_enabled -eq 0 ]; then
		uci_set firewall ${ipsec_esp} enabled 1
	fi

	local ipsec_ika_enabled=$(uci_get firewall ${ipsec_ika} enabled)
	if [ $ipsec_ika_enabled -eq 0 ]; then
		uci_set firewall ${ipsec_ika} enabled 1
	fi

	local ipsec_nat_enabled=$(uci_get firewall ${ipsec_nat} enabled)
	if [ $ipsec_nat_enabled -eq 0 ]; then
		uci_set firewall ${ipsec_nat} enabled 1
	fi

	local auth_header_enabled=$(uci_get firewall ${auth_header} enabled)
	if [ $auth_header_enabled -eq 0 ]; then
		uci_set firewall ${auth_header} enabled 1
	fi
	uci_commit firewall

else
	echo "Copy wan/l2tp client configurations..."
	cp /etc/ppp/options.xl2tpd.default /etc/ppp/options.xl2tpd
	cp /etc/xl2tpd/xl2tpd.conf.default /etc/xl2tpd/xl2tpd.conf
fi
