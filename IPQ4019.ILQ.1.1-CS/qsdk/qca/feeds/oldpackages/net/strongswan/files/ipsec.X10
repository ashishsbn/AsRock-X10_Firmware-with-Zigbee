# ASRock X10 IPSec/L2TP settings
#!/bin/sh

. /lib/config/uci.sh

# get X10 IPSec information from config
local secret=$(uci_get ipsec strongswan secret)
local total=$(uci_get ipsec strongswan total)

if [ "${secret}" != "" ]; then
	mkdir -p /var/etc
	mkdir -p /var/etc/ppp
	rm -f /var/etc/ipsec.secrets
	rm -f /var/etc/ppp/chap-secrets

	echo "# /etc/ipsec.secrets - strongSwan IPsec secrets file" > /var/etc/ipsec.secrets
	echo "#USERNAME  PROVIDER  PASSWORD  IPADDRESS" > /var/etc/ppp/chap-secrets
	echo >&2 "secret: ${secret}"
	echo "%any : PSK \"${secret}\"" >> /var/etc/ipsec.secrets
	# get username and password
	local username password
	for key_index in $(seq 0 $((total-1)))
	do
		username=$(uci_get ipsec @account[${key_index}] username)
		password=$(uci_get ipsec @account[${key_index}] password)
		echo >&2 "username: ${username}, password: ${password}"
		# Add escape for some special characters.
		username=$(echo $username | sed 's/\\/\\\\/g;s/\"/\\\"/g')
		password=$(echo $password | sed 's/\\/\\\\/g;s/\"/\\\"/g')
		echo "\"${username}\"	l2tpd	\"${password}\"	10.1.20.128/25" >> /var/etc/ppp/chap-secrets
	done

	[ -L /etc/ipsec.secrets ] || ln -nsf /var/etc/ipsec.secrets /etc/ipsec.secrets
	[ -L /etc/ppp/chap-secrets ] || ln -nsf /var/etc/ppp/chap-secrets /etc/ppp/chap-secrets
	
	# restart xl2tpd
	/etc/init.d/xl2tpd restart
fi

# Replace LAN IP
local strongswan_config="/var/etc/strongswan.conf"
#local lan_ipaddr=$(uci_get network lan ipaddr)
#local lan_subnet=`echo $lan_ipaddr | cut -d. -f1-3`
#echo >&2 "lan_subnet: ${lan_subnet}"
if [ -f /etc/strongswan.conf.template ]; then
#	echo >&2 "Repalce LAN IP of ${strongswan_config}"

	mkdir -p /var/etc
	rm -f ${strongswan_config}
	cp /etc/strongswan.conf.template ${strongswan_config}

#	sed -i '/dns1 /c\	dns1 = '"${lan_ipaddr}" "${strongswan_config}"
#	sed -i '/server /c\			server = '"${lan_subnet}"'.255' "${strongswan_config}"

	[ -L /etc/strongswan.conf ] || ln -nsf /var/etc/strongswan.conf /etc/strongswan.conf
fi

local ipsec_config="/var/etc/ipsec.conf"
if [ -f /etc/ipsec.conf.template ]; then
#	echo >&2 "Repalce LAN IP of ${ipsec_config}"

	mkdir -p /var/etc
	rm -f ${ipsec_config}
	cp /etc/ipsec.conf.template ${ipsec_config}
#	sed -i '/rightsubnet.*24/c\	rightsubnet='"${lan_subnet}"'.0/24' "${ipsec_config}"

	[ -L /etc/ipsec.conf ] || ln -nsf /var/etc/ipsec.conf /etc/ipsec.conf
fi
