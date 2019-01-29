#
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#

local pid
WPS_RUNNING_FILE="/var/run/wps_is_running"

if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	[ -r /var/run/wifi-wps-enhc-extn.conf ] && exit 0
	op_mode=$(uci get system.@system[0].op_mode)
	if [ "$op_mode" == "0" -o "$op_mode" == "2" ]; then
		exit 0
	fi
	if [ -f $WPS_RUNNING_FILE ]; then
		exit 0
	else
		touch $WPS_RUNNING_FILE
	fi

	echo "" > /dev/console
	echo "WPS PUSH BUTTON EVENT DETECTED (wpa_supplicant)" > /dev/console

	sh /usr/sbin/wps_sta $op_mode &
fi
