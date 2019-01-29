#
# Copyright (c) 2014 Qualcomm Atheros, Inc.
#
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.
#

. /lib/functions.sh

qwrap_config() {
	local device=$1
	local ifidx=0
	local radioidx=${device#wifi}

	config_get disabled "$device" disabled 0
	config_get qwrap_enable "$device" qwrap_enable 0
	config_get qwrap_dbdc_enable "$device" qwrap_dbdc_enable 0

	[ "$disabled" -eq 0 ] || return
	[ "$qwrap_enable" -gt 0 -o "$qwrap_dbdc_enable" -gt 0 ] || return

	[ "$qwrap_enable" -gt 0 ] && qwrap_enabled=1 && append wrap_device "-D $device"

	config_get vifs $device vifs

	for vif in $vifs; do
		local vifname

		config_get vifname "$vif" ifname

		if [ -n $vifname ]; then
			[ $ifidx -gt 0 ] && vifname="ath${radioidx}$ifidx" || vifname="ath${radioidx}"
		fi

		config_get mode "$vif" mode

		case "$mode" in
			wrap)
				append wrapd_ifname "-a $vifname"
			;;
			ap)
				config_get qwrap_ap "$vif" qwrap_ap 0
				[ "$qwrap_ap" -gt 0 ] && append wrapd_ifname "-a $vifname"
			;;
			sta)
				[ "$qwrap_enable" -gt 0 ] || continue

		                append sta_ifname "-p $vifname"
				wrap_sta_macaddr="$(cat /sys/class/net/${vifname}/address)"

				config_load network
				net_cfg="$(find_net_config "$vif")"
				config_set "$net_cfg" macaddr "$wrap_sta_macaddr"

				[ -z "$net_cfg" ] || {
                                        if [ $sta_mac_set_to_bridge -eq 0 ]; then
                                                echo "QWRAP-Adding mac address to bridge sta_mac" $wrap_sta_macaddr
                                                sta_mac_set_to_bridge=1
                                                bridge="$(bridge_interface "$net_cfg")"
                                                ifconfig "$bridge" hw ether "$wrap_sta_macaddr"
                                        else
                                                echo "QWRAP-Already bridge mac is set ignoring mac" $wrap_sta_macaddr
                                        fi
				}
                                tmp_conf_file=$(cat "/tmp/qwrap_conf_filename-$vifname.conf")
                                append wrapd_conf_file "-c $tmp_conf_file"
			;;
		esac

		ifidx=$(($ifidx + 1))

	done

	[ "$qwrap_enable" -gt 0 ] || return

	config_get_bool ap_isolation_enabled $device ap_isolation_enabled 0

	if [ $ap_isolation_enabled -ne 0 ]; then
		iso="-I"
		echo '1 1' > /proc/wrap$radioidx
	fi

	config_get_bool wrapd_vma $device wrapd_vma 0
	[ $wrapd_vma -ne 0 ] && append wrapd_vma_conf "-v /etc/ath/wrap-vma-$device.conf"
	config_get qwrap_eth_name "$device" qwrap_eth_name "eth1"
	config_get qwrap_br_name "$device" qwrap_br_name $bridge
	config_get qwrap_sta_limit "$device" qwrap_sta_limit 20
	config_get qwrap_poll_timer "$device" qwrap_poll_timer 1
	config_get qwrap_eth_sta_del_en "$device" qwrap_eth_sta_del_en 0
	config_get qwrap_eth_sta_add_en "$device" qwrap_eth_sta_add_en 0
	append wrapd_ctrl_interface "-g /var/run/wrapd-global-$device"
	wpa_supplicant_global_ctrl_iface="/var/run/wpa_supplicantglobal"
	wrapd_pid="/var/run/wrapd-$device.pid"
	bridge_conf_file="-b $qwrap_br_name -i $qwrap_eth_name
		-l $qwrap_sta_limit -t $qwrap_poll_timer
		-e $qwrap_eth_sta_add_en -r $qwrap_eth_sta_del_en"
}

qwrap_setup() {
	local qwrap_enabled=0 iso="" wrapd_vma_conf="" wrap_device sta_mac_set_to_bridge=0 tmp_conf_file="" 

	config_load wireless

	config_foreach qwrap_config wifi-device

	[ "$qwrap_enabled" -gt 0 ] || return

	wrapd ${iso} -P $wrapd_pid $wrap_device $wrapd_conf_file $wrapd_ifname $sta_ifname $wrapd_vma_conf $wrapd_ctrl_interface -w $wpa_supplicant_global_ctrl_iface $bridge_conf_file &
}

qwrap_unconfig() {
	local device=$1
	local ifidx=0
	local radioidx=${device#wifi}

	[ -f "/var/run/wrapd-$device.pid" ] &&
		kill "$(cat "/var/run/wrapd-$device.pid")"

	[ -f "/var/run/wpa_supplicant-qwrap-$device.pid" ] &&
		kill "$(cat "/var/run/wpa_supplicant-qwrap-$device.pid")"

	[ -f "/var/run/wrapd-global-$device" ] &&
		rm -rf /var/run/wrapd-global-$device

	[ -f "/var/run/wpa_supplicant-global-$device" ] &&
		rm -rf /var/run/wpa_supplicant-global-$device

	config_get vifs $device vifs

	for vif in $vifs; do
		local vifname

		config_get vifname "$vif" ifname

		if [ -n $vifname ]; then
			[ $ifidx -gt 0 ] && vifname="ath${radioidx}$ifidx" || vifname="ath${radioidx}"
		fi

		config_get mode "$vif" mode

		case "$mode" in
			sta)
				config_load network
				net_cfg="$(find_net_config "$vif")"

				[ -z "$net_cfg" ] || {

					bridge="$(bridge_interface "$net_cfg")"
					cd /sys/class/net/${bridge}/brif

					for eth in $(ls -d eth* 2>&-); do
						br_macaddr="$(cat /sys/class/net/${eth}/address)"
						[ -n "$br_macaddr" ] && break
					done

					ifconfig "$bridge" hw ether "$br_macaddr"
				}
	                        [ -f "/tmp/qwrap_conf_filename-$vifname.conf" ] &&
		                        rm /tmp/qwrap_conf_filename-$vifname.conf
			;;
		esac

		ifidx=$(($ifidx + 1))

	done
}

qwrap_teardown() {
	config_load wireless

	config_foreach qwrap_unconfig wifi-device
}
