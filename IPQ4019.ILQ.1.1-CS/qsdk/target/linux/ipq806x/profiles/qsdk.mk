#
# Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
#

NSS_COMMON:= \
	kmod-qca-nss-drv \
	kmod-qca-nss-gmac \
	kmod-qca-edma

NSS_STANDARD:= \
	qca-nss-fw-retail \

NSS_ENTERPRISE:= \
	qca-nss-fw-enterprise \
        qca-nss-fw-enterprise_custA \
	qca-nss-fw-enterprise_custC \

NSS_MACSEC:= \
	kmod-qca-nss-macsec \
	qca-wpa-supplicant-10.4-macsec

QCA_ECM:= kmod-qca-nss-ecm
QCA_ECM_PREMIUM:= kmod-qca-nss-ecm-premium
QCA_ECM_ENTERPRISE:= kmod-qca-nss-ecm-noload

NSS_CLIENTS:= kmod-qca-nss-drv-qdisc kmod-qca-nss-drv-profile kmod-qca-nss-drv-tun6rd kmod-qca-nss-drv-tunipip6 kmod-qca-nss-drv-l2tpv2 kmod-qca-nss-drv-pptp
NSS_CLIENTS_ENTERPRISE:= kmod-qca-nss-drv-qdisc kmod-qca-nss-drv-profile

NSS_CRYPTO:= kmod-qca-nss-crypto kmod-qca-nss-cfi kmod-qca-nss-drv-ipsecmgr

HW_CRYPTO:= kmod-crypto-qcrypto

SHORTCUT_FE:= kmod-shortcut-fe kmod-shortcut-fe-cm kmod-shortcut-fe-drv
QCA_RFS:= kmod-qca-rfs

SWITCH_SSDK_PKGS:= kmod-qca-ssdk-hnat qca-ssdk-shell  swconfig
SWITCH_OPEN_PKGS:= kmod-switch-ar8216 swconfig

WIFI_OPEN_PKGS:= kmod-ath9k kmod-ath10k wpad hostapd-utils \
		 kmod-art2-netlink sigma-dut-open wpa-cli qcmbr-10.4-netlink

WIFI_10_4_PKGS:=kmod-qca-wifi-10.4-unified-profile qca-wifi-fw-hw5-10.4-asic \
	qca-hostap-10.4 qca-hostapd-cli-10.4 qca-wpa-supplicant-10.4 \
	qca-wpa-cli-10.4 qca-spectral-10.4 qca-wpc-10.4 sigma-dut-10.4 \
	qcmbr-10.4 qca-wrapd-10.4 qca-wapid qca-acfg-10.4 whc whc-ui

WIL6210_PKGS:=kmod-wil6210 wigig-firmware iwinfo

WIFI_10_4_FW_PKGS:=qca-wifi-fw-hw2-10.4-asic qca-wifi-fw-hw4-10.4-asic

OPENWRT_STANDARD:= \
	luci openssl-util

STORAGE:=kmod-scsi-core kmod-usb-storage \
	kmod-fs-msdos kmod-fs-ntfs kmod-fs-vfat \
	kmod-nls-cp437 kmod-nls-iso8859-1 \
	mdadm ntfs-3g e2fsprogs fdisk mkdosfs

NETWORKING:=mcproxy -dnsmasq dnsmasq-dhcpv6 bridge ip-full trace-cmd \
	rp-pppoe-relay iptables-mod-extra iputils-tracepath iputils-tracepath6 \
	kmod-ipt-nathelper-extra kmod-ipt-nathelper-rtsp \
	luci-app-upnp luci-app-radvd luci-app-ddns luci-proto-ipv6

CD_ROUTER:=kmod-ipt-ipopt kmod-bonding kmod-nat-sctp kmod-ipt-conntrack-qos \
	arptables ds-lite radvd 6rd wide-dhcpv6-client ddns-scripts xl2tpd \
	quagga quagga-ripd quagga-zebra quagga-watchquagga quagga-vtysh \
	kmod-ipv6 ip6tables iptables-mod-ipsec iptables-mod-filter \
	rp-pppoe-server isc-dhcp-relay-ipv4 isc-dhcp-relay-ipv6 ppp-mod-pptp

QOS:=luci-app-qos kmod-sched

MAP_PKGS:=map-t

HYFI:=hyfi hyfi-ui      # no PLC support
HYFI_PLC:=hyfi-plc hyfi-ui  # with PLC support

AQ_PHY:=kmod-aq_phy kmod-qca_85xx_sw aq-fw-download
#These packages depend on SWITCH_SSDK_PKGS
IGMPSNOOING_RSTP:=rstp qca-mcs-apps

IPSEC:=openswan kmod-ipsec kmod-ipsec4 kmod-ipsec6

TEST_TOOLS:=sysstat devmem2 ethtool i2c-tools

UTILS:=tftp-hpa pure-ftpd qca-thermald-10.4 file luci-app-samba iperf \
	rng-tools

BLUETOOTH:=bluez kmod-ath3k

FAILSAFE:= kmod-bootconfig

AUDIO:=kmod-sound-soc-ipq40xx alsa

VIDEO:=kmod-qpic_panel_ertft

define Profile/QSDK_Open
	NAME:=Qualcomm-Atheros SDK Open Profile
	PACKAGES:=$(OPENWRT_STANDARD) $(NSS_COMMON) $(NSS_STANDARD) $(SWITCH_OPEN_PKGS) \
		$(WIFI_OPEN_PKGS) $(STORAGE) $(CD_ROUTER) $(NETWORKING) $(UTILS) \
		$(BLUETOOTH) $(QCA_ECM) $(NSS_CRYPTO) $(NSS_CLIENTS) $(QOS) $(TEST_TOOLS) alsa
endef

define Profile/QSDK_Open/Description
	QSDK Open package set configuration.
	Enables wifi open source packages
endef
$(eval $(call Profile,QSDK_Open))

define Profile/QSDK_Premium
	NAME:=Qualcomm-Atheros SDK Premium Profile
	PACKAGES:=$(OPENWRT_STANDARD) $(NSS_COMMON) $(NSS_STANDARD) $(SWITCH_SSDK_PKGS) \
		$(WIFI_10_4_PKGS) $(WIFI_10_4_FW_PKGS) $(STORAGE) $(CD_ROUTER) \
		$(NETWORKING) $(UTILS) $(SHORTCUT_FE) $(BLUETOOTH) $(HW_CRYPTO) $(QCA_RFS) \
		$(AUDIO) $(VIDEO) $(IGMPSNOOING_RSTP) $(IPSEC) $(QOS) $(QCA_ECM_PREMIUM) \
		$(NSS_MACSEC) $(TEST_TOOLS) $(NSS_CRYPTO) $(NSS_CLIENTS) $(WIL6210_PKGS) pm-utils \
		$(MAP_PKGS) $(HYFI) $(AQ_PHY) $(FAILSAFE) kmod-gobinet
endef

define Profile/QSDK_Premium/Description
	QSDK Premium package set configuration.
	Enables qca-wifi 10.4 packages
endef

$(eval $(call Profile,QSDK_Premium))

define Profile/QSDK_Standard
	NAME:=Qualcomm-Atheros SDK Standard Profile
	PACKAGES:=$(OPENWRT_STANDARD) $(NSS_COMMON) $(NSS_STANDARD) $(SWITCH_SSDK_PKGS) \
		$(WIFI_10_4_PKGS) $(STORAGE) $(SHORTCUT_FE) $(HW_CRYPTO) $(QCA_RFS) \
		$(IGMPSNOOING_RSTP) $(NETWORKING) $(QOS) $(UTILS) $(TEST_TOOLS) pm-utils \
		$(FAIL_SAFE)
endef

define Profile/QSDK_Standard/Description
	QSDK Standard package set configuration.
	Enables qca-wifi 10.4 packages
endef

$(eval $(call Profile,QSDK_Standard))

define Profile/QSDK_Enterprise
	NAME:=Qualcomm-Atheros SDK Enterprise Profile
	PACKAGES:=$(OPENWRT_STANDARD) $(NSS_COMMON) $(NSS_ENTERPRISE) $(SWITCH_SSDK_PKGS) \
		$(WIFI_10_4_PKGS) $(WIFI_10_4_FW_PKGS) $(STORAGE) $(HW_CRYPTO) $(QCA_RFS) \
		$(IGMPSNOOING_RSTP) $(NETWORKING) $(QOS) $(UTILS) $(TEST_TOOLS) pm-utils \
		$(QCA_ECM_ENTERPRISE) $(NSS_CLIENTS_ENTERPRISE) $(NSS_MACSEC) $(NSS_CRYPTO) \
		$(IPSEC) $(CD_ROUTER)
endef

define Profile/QSDK_Enterprise/Description
	QSDK Enterprise package set configuration.
	Enables qca-wifi 10.4 packages
endef

$(eval $(call Profile,QSDK_Enterprise))

define Profile/QSDK_ASRock_X10
	NAME:=Qualcomm-Atheros SDK for ASRock X10 profile
	PACKAGES:=$(OPENWRT_STANDARD) $(NSS_COMMON) $(NSS_STANDARD) $(SWITCH_SSDK_PKGS) \
		$(WIFI_10_4_PKGS) $(STORAGE) $(CD_ROUTER) \
		$(NETWORKING) $(UTILS) $(SHORTCUT_FE) $(BLUETOOTH) $(HW_CRYPTO) $(QCA_RFS) \
		$(IGMPSNOOING_RSTP) $(IPSEC) $(QOS) $(QCA_ECM_PREMIUM) \
		$(NSS_MACSEC) $(TEST_TOOLS) $(NSS_CRYPTO) $(NSS_CLIENTS) pm-utils \
		$(MAP_PKGS) $(HYFI) $(AQ_PHY) $(FAILSAFE) kmod-gobinet
endef

define Profile/QSDK_ASRock_X10/Description
	QSDK ASRock_X10 package set configuration.
	Enables qca-wifi 10.4 packages
endef

$(eval $(call Profile,QSDK_ASRock_X10))

