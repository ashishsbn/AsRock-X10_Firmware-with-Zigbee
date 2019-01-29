# ASRock X10 Custom Domain for VPN server
#!/bin/sh

. /lib/config/uci.sh

local openvpn_enable=$(uci_get openvpn sample_server enabled)
local ipsec_open=$(uci_get ipsec.strongswan.enabled)
local lan_ipaddr=$(uci_get network.lan.ipaddr)
local dnsmasq_config="/etc/dnsmasq.conf"
sed -i '/listen-address/d' "${dnsmasq_config}"
if [ $openvpn_enable == "1" ] && [ $ipsec_open == "1" ]; then
	echo "Both VPNs are active, enabling Custom Domain"
	echo "listen-address=127.0.0.1,${lan_ipaddr},10.1.20.1,10.8.0.1" >> "${dnsmasq_config}"
elif [ $openvpn_enable == "0" ] && [ $ipsec_open == "1" ]; then
	echo "IPSec VPN is active, enabling Custom Domain"
	echo "listen-address=127.0.0.1,${lan_ipaddr},10.1.20.1" >> "${dnsmasq_config}"
elif [ $openvpn_enable == "1" ] && [ $ipsec_open == "0" ]; then
	echo "OpenVPN is active, enabling Custom Domain"
	echo "listen-address=127.0.0.1,${lan_ipaddr},10.8.0.1" >> "${dnsmasq_config}"
fi
