#!/bin/sh
# X10 customized firewall

# Delete Guided Rule
iptables -t nat -D zone_wan_prerouting -m comment --comment Go_To_PORT_FORWARDING -j PORT_FORWARDING
iptables -t nat -D zone_wan_prerouting -m comment --comment Go_To_PORT_TRIGGER_WAN_PREROUTING -j PORT_TRIGGER_WAN_PREROUTING
iptables -t nat -D zone_lan_prerouting -m comment --comment Go_To_PORT_TRIGGER_LAN_PREROUTING -j PORT_TRIGGER_LAN_PREROUTING
iptables -t filter -D zone_wan_forward -m comment --comment Go_To_PORT_TRIGGER_WAN_FORWARD -j PORT_TRIGGER_WAN_FORWARD
iptables -t nat -D zone_wan_prerouting -m comment --comment Go_To_DMZ -j DMZ
iptables -D forwarding_rule -i br-lan -p tcp -m comment --comment Go_To_PARENTAL_CONTROL -j PARENTAL_CONTROL
iptables -t nat -D prerouting_lan_rule -m comment --comment Go_To_WEB_AUTO_OPEN -j WEB_AUTO_OPEN

# Flush Chains
iptables -t nat -F PORT_FORWARDING
iptables -t nat -F PORT_TRIGGER_WAN_PREROUTING
iptables -t nat -F PORT_TRIGGER_LAN_PREROUTING
iptables -t filter -F PORT_TRIGGER_WAN_FORWARD
iptables -t nat -F DMZ
iptables -F PARENTAL_CONTROL
iptables -F URL_FILTER
iptables -F KEYWORD_FILTER
iptables -t nat -F WEB_AUTO_OPEN

# Delete Chains
iptables -t nat -X PORT_FORWARDING
iptables -t nat -X PORT_TRIGGER_WAN_PREROUTING
iptables -t nat -X PORT_TRIGGER_LAN_PREROUTING
iptables -t filter -X PORT_TRIGGER_WAN_FORWARD
iptables -t nat -X DMZ
iptables -X URL_FILTER
iptables -X KEYWORD_FILTER
iptables -X PARENTAL_CONTROL
iptables -t nat -X WEB_AUTO_OPEN

# New Chains
iptables -t nat -N PORT_FORWARDING
iptables -t nat -N PORT_TRIGGER_WAN_PREROUTING
iptables -t nat -N PORT_TRIGGER_LAN_PREROUTING
iptables -t filter -N PORT_TRIGGER_WAN_FORWARD
iptables -t nat -N DMZ
iptables -N PARENTAL_CONTROL
iptables -N URL_FILTER
iptables -N KEYWORD_FILTER
iptables -t nat -N WEB_AUTO_OPEN

# Add rules for new chains
iptables -t nat -A zone_wan_prerouting -m comment --comment Go_To_PORT_FORWARDING -j PORT_FORWARDING
iptables -t nat -A zone_wan_prerouting -m comment --comment Go_To_PORT_TRIGGER_WAN_PREROUTING -j PORT_TRIGGER_WAN_PREROUTING
iptables -t nat -A zone_lan_prerouting -m comment --comment Go_To_PORT_TRIGGER_LAN_PREROUTING -j PORT_TRIGGER_LAN_PREROUTING
iptables -t filter -I zone_wan_forward 2 -m comment --comment Go_To_PORT_TRIGGER_WAN_FORWARD -j PORT_TRIGGER_WAN_FORWARD
iptables -t nat -A zone_wan_prerouting -m comment --comment Go_To_DMZ -j DMZ
iptables -A forwarding_rule -i br-lan -p tcp -m comment --comment Go_To_PARENTAL_CONTROL -j PARENTAL_CONTROL
iptables -A PARENTAL_CONTROL -m comment --comment Go_To_URL_FILTER -j URL_FILTER
iptables -A PARENTAL_CONTROL -m comment --comment Go_To_KEYWORD_FILTER -j KEYWORD_FILTER
iptables -t nat -A prerouting_lan_rule -m comment --comment Go_To_WEB_AUTO_OPEN -j WEB_AUTO_OPEN

#PORT_TRIGGER
if [ "$(uci get firewall.port_trigger.enabled)" == "1" ]; then
	i=0;
	while [ 1 ] ; do
		j=$(printf '%03d' $i)
		pt_config_name="pt"$j
		src_ports=$(uci get firewall.$pt_config_name.src_dport)
		src_ports2=`echo $src_ports | sed -e "s/:/-/g"`
		if [ ${#src_ports} == 0 ]; then
				break;
		fi
		name=$(uci get firewall.$pt_config_name.name)
		trigger_ports=$(uci get firewall.$pt_config_name.trigger_port)
		protocol=$(uci get firewall.$pt_config_name.proto)
		if [ $protocol != "tcpudp" ]; then
			iptables -t nat -A PORT_TRIGGER_WAN_PREROUTING -p $protocol --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type dnat
			iptables -A PORT_TRIGGER_WAN_FORWARD -p $protocol --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type in
			iptables -t nat -A PORT_TRIGGER_LAN_PREROUTING -i br-lan -p $protocol --dport $trigger_ports -m comment --comment "$name" -j TRIGGER --trigger-type out --trigger-proto $protocol --trigger-match $src_ports2 --trigger-relate $src_ports2
		else
			iptables -t nat -A PORT_TRIGGER_WAN_PREROUTING -p tcp --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type dnat
			iptables -A PORT_TRIGGER_WAN_FORWARD -p tcp --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type in
			iptables -t nat -A PORT_TRIGGER_LAN_PREROUTING -i br-lan -p tcp --dport $trigger_ports -m comment --comment "$name" -j TRIGGER --trigger-type out --trigger-proto tcp --trigger-match $src_ports2 --trigger-relate $src_ports2
			iptables -t nat -A PORT_TRIGGER_WAN_PREROUTING -p udp --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type dnat
			iptables -A PORT_TRIGGER_WAN_FORWARD -p udp --dport $src_ports -m comment --comment "$name" -j TRIGGER --trigger-type in
			iptables -t nat -A PORT_TRIGGER_LAN_PREROUTING -i br-lan -p udp --dport $trigger_ports -m comment --comment "$name" -j TRIGGER --trigger-type out --trigger-proto udp --trigger-match $src_ports2 --trigger-relate $src_ports2
		fi
		i=`expr $i + 1`;
	done
fi

# PORT_FORWARDING
if [ "$(uci get firewall.port_forwarding.enabled)" == "1" ]; then
	i=0;
	while [ 1 ] ; do
		j=$(printf '%03d' $i)
		pf_config_name="pf"$j
		src_ports=$(uci get firewall.$pf_config_name.src_dport)
		if [ ${#src_ports} == 0 ]; then
				break;
		fi
		name=$(uci get firewall.$pf_config_name.name)
		dest_ip=$(uci get firewall.$pf_config_name.dest_ip)
		dest_ports=$(uci get firewall.$pf_config_name.dest_port)
		protocol=$(uci get firewall.$pf_config_name.proto)
		delimiter=","
		if [ "${src_ports/$delimiter}" == "$src_ports" ] ; then
			dest_ports2=`echo $dest_ports | sed -e "s/:/-/g"`
			if [ $protocol != "tcpudp" ]; then
				iptables -t nat -A PORT_FORWARDING -p $protocol -m $protocol --dport $src_ports -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dest_ports2
			else
				iptables -t nat -A PORT_FORWARDING -p tcp -m tcp --dport $src_ports -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dest_ports2
				iptables -t nat -A PORT_FORWARDING -p udp -m udp --dport $src_ports -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dest_ports2
			fi
		else
			ports=$(echo $src_ports | tr "," "\n")
			if [ $protocol != "tcpudp" ]; then
				for port in $ports
				do
					dport=`echo $port | sed -e "s/:/-/g"`
					iptables -t nat -A PORT_FORWARDING -p $protocol -m $protocol --dport $port -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dport
				done
			else
				for port in $ports
				do
					dport=`echo $port | sed -e "s/:/-/g"`
					iptables -t nat -A PORT_FORWARDING -p tcp -m tcp --dport $port -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dport
					iptables -t nat -A PORT_FORWARDING -p udp -m udp --dport $port -m comment --comment "$name" -j DNAT --to-destination $dest_ip:$dport
				done
			fi
		fi
		i=`expr $i + 1`;
	done
fi

# DMZ
if [ $(uci get firewall.dmz.enabled) == "1" ]; then
	iptables -t nat -A DMZ -m comment --comment DMZ -j DNAT --to-destination $(uci get firewall.dmz.dest_ip)
fi

# Parental Control
# URL Filter
if [ $(uci get firewall.parental_control.url_filter) == "1" ]; then
	i=0;
	while [ 1 ] ; do
		j=$(printf '%03d' $i)
		url_config_name="url"$j
		url=$(uci get firewall.$url_config_name.url)
		if [ ${#url} == 0 ]; then
			break;
		fi
		iptables -A URL_FILTER -m weburl --contains "$url" -j reject
		i=`expr $i + 1`;
	done
fi
# Keyword Filter
if [ $(uci get firewall.parental_control.keyword_filter) == "1" ]; then
	i=0;
	while [ 1 ] ; do
		j=$(printf '%03d' $i)
		keyword_config_name="keyword"$j
		keyword=$(uci get firewall.$keyword_config_name.keyword)
		if [ ${#keyword} == 0 ]; then
			break;
		fi
		iptables -A KEYWORD_FILTER -m string --icase --algo bm --string "$keyword" -j reject
		i=`expr $i + 1`;
	done
fi

# vpn firewall settings
net=$(uci get firewall.@zone[1].network)
vpn_open=0

for x in `echo $net | tr ";" "\n"`
do
	if [ $x == "vpn" ]; then
		vpn_proto=$(uci get network.vpn.proto)
		if [ $vpn_proto == "pptp" ]; then
			vpn_open=1
		fi
	fi
done

if [ $vpn_open == "1" ]; then
	iptables -D zone_wan_input -j zone_wan_src_REJECT

	iptables -A zone_wan_input -i $(uci get network.vpn.ifname) -j ACCEPT
	iptables -A zone_wan_input -i $(uci get network.wan.ifname) -j ACCEPT
	iptables -A zone_wan_input -j zone_wan_src_REJECT
fi

# WEB Auto Open
factory_default=$(uci get system.@system[0].factory_default)
if [ $factory_default == "1" ]; then
	lan_ip=$(uci get network.lan.ipaddr)
	#iptables -t nat -A WEB_AUTO_OPEN -p tcp -m tcp --dport 80 -j DNAT --to-destination $lan_ip
	#iptables -t nat -A WEB_AUTO_OPEN -p tcp -m tcp --dport 443 -j DNAT --to-destination $lan_ip
fi

